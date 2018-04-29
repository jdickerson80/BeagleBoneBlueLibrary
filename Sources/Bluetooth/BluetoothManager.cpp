#include "BluetoothManager.h"

#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "ThreadHelper.h"
#include "BluetoothInterface.h"
#include <signal.h>
#include <arpa/inet.h>
namespace LibBBB {
namespace Bluetooth {

Manager::Manager( const std::string& peerAddress
				  , const std::string& localAddress
				  , uint32_t receiveMessageSize
				  , uint32_t sendMessageSize
				  , uint32_t pollRate
				  , Interface* listenerObject /* = NULL */
				  , stateChange listenerMethod /* = NULL */ )
	: _callbackThreadRunning( false )
	, _receiveThreadRunning( false )
	, _sendThreadRunning( false )
	, _setupThreadRunning( false )
	, _sendCondition( false )
	, _socket( 0 )
	, _pollRate( pollRate )
	, _receiveMessageSize( receiveMessageSize )
	, _sendMessageSize( sendMessageSize )
	, _currentState( State::Connecting )
	, _listenerObject( listenerObject )
	, _listenerMethod( listenerMethod )
{
	signal( SIGPIPE, SIG_IGN );
	if ( listenerObject )
	{
		LibBBB::ThreadHelper::startDetachedThread( &_callbackThread, handleCallbacks, &_callbackThreadRunning, static_cast< void* >( this ) );
	}

	_localAddress = new sockaddr_in;
	*_localAddress = { AF_INET, htons( 3333 ), htonl( INADDR_ANY ) };


	// create the send and receive buffers
	_receiveMessageBuffer = (uint8_t*)malloc( _receiveMessageSize * sizeof( uint8_t ) );
	_sendMessageBuffer = (uint8_t*)malloc( _sendMessageSize * sizeof( uint8_t ) );

	// init the pthread and conditional variables
	_sendMutex = PTHREAD_MUTEX_INITIALIZER;
	_sendConditionalVariable = PTHREAD_COND_INITIALIZER;

	// start the setup thread
	LibBBB::ThreadHelper::startDetachedThread( &_setupThread, setupConnection, &_setupThreadRunning, static_cast< void* >( this ) );
}

Manager::~Manager()
{
	// check whether the threads are running, and if so,
	// shut them down

	if ( this->_callbackThreadRunning )
	{
		pthread_cancel( this->_callbackThread );
	}

	if ( this->_receiveThreadRunning )
	{
		pthread_cancel( this->_receiveThread );
	}

	if ( this->_sendThreadRunning )
	{
		pthread_cancel( this->_sendThread );
	}

	if ( this->_setupThreadRunning )
	{
		pthread_cancel( this->_setupThread );
	}

	// clost the socket
	close( _socket );

	// destroy the mutex
	pthread_mutex_destroy( &_sendMutex );

	// delete all the variables that were created by new
	delete _localAddress;
	delete _peerAddress;
	delete _receiveMessageBuffer;
	delete _sendMessageBuffer;
}

ssize_t Manager::sendData( const uint8_t* const data )
{
	///@todo remove the return or make it usefull

	// if the peer device is not connected, just return
	if ( _currentState != State::Connected )
	{
		printf("send returned -1\n");
		return -1;
	}

	// lock the mutex
	pthread_mutex_lock( &_sendMutex );

	// copy the desired data to be sent to the buffer
	memcpy( (void*)_sendMessageBuffer, data, _sendMessageSize );

	// set the flag to true for the conditional variable
	_sendCondition = true;

	// unlock the mutex, and signal the contitional variable
	pthread_mutex_unlock( &_sendMutex );
	pthread_cond_signal( &_sendConditionalVariable );
	return 1;
}

const uint8_t* const Manager::receiveData()
{
	// if the peer device is not connected, just return null
	if ( _currentState != State::Connected )
	{
		printf("receive returned\n");
		return NULL;
	}

	// lock the mutex
//	pthread_mutex_lock( &_receiveMutex );

	const uint8_t * const data = _receiveMessageBuffer;

	// unlock the mutex
//	pthread_mutex_unlock( &_receiveMutex );

	// otherwise return the pointer to the send buffer
	return data;
}

bool Manager::isConnected() const
{
	return _currentState == State::Connected;
}

void* Manager::receiveMessage( void* input )
{
	// get local variables
	Manager* manager = static_cast< Manager* >( input );
	ssize_t messageSize = manager->_receiveMessageSize;
	void* receiveMessageBuffer = (void*)manager->_receiveMessageBuffer;
	uint8_t buffer[ messageSize ];

	while ( manager->_receiveThreadRunning )
	{
		// recieve the message
		ssize_t result = recv( manager->_socket, buffer, messageSize, 0 );

		// check if the socket is still connected
		if ( errno == ENOTCONN || result != messageSize )
		{
			break;
		}

		// lock the mutex
//		pthread_mutex_lock( mutex );

		// copy the desired data to be sent to the buffer
		memcpy( receiveMessageBuffer, buffer, messageSize );

		// unlock the mutex
//		pthread_mutex_unlock( &manager->_receiveMutex );

	}

	if ( manager->_sendThreadRunning )
	{
		pthread_cancel( manager->_sendThread );
	}

	pthread_exit( NULL );
	return NULL;
}

void* Manager::sendMessage( void* input )
{
	// init the local variables
	Manager* manager = static_cast< Manager* >( input );
	int32_t messageSize = (int32_t)manager->_sendMessageSize;
	pthread_cond_t* conditionalVariable = &manager->_sendConditionalVariable;
	pthread_mutex_t* mutex = &manager->_sendMutex;
	uint32_t pollRate = manager->_pollRate;
	uint8_t* sendMessageBuffer = manager->_sendMessageBuffer;

	while ( manager->_sendThreadRunning )
	{
		// lock the mutex
		pthread_mutex_lock( mutex );

		// wait until the condition has been met
		while ( !manager->_sendCondition )
		{
			// wait for the condition
			pthread_cond_wait( conditionalVariable, mutex );
		}

		// send the data
		ssize_t result = send( manager->_socket, sendMessageBuffer, messageSize, 0 );

		// set the condition flag to false
		manager->_sendCondition = false;

		// unlock the mutex
		pthread_mutex_unlock( mutex );

		// check for send errors
		if ( result < 0 && errno != 0 )
		{
			break;
		}

		// check if all of the data was sent
		if ( result != messageSize )
		{
			// it was not, so go back to the send function to finish
			break;
		}

		// sleep for the receive/send rate
		usleep( pollRate );
	}

	if ( manager->_receiveThreadRunning )
	{
		pthread_cancel( manager->_receiveThread );
	}

	pthread_exit( NULL );
	return NULL;
}


void* Manager::setupConnection( void* input )
{
	// init the local variables
	Manager* manager = static_cast< Manager* >( input );
	unsigned int opt = sizeof( *manager->_peerAddress );
	int returnValue = 0;

	// clear the variables
	int localSocket;

	// get a local copy of the addresses
	sockaddr_in localCopy = *manager->_localAddress;
	sockaddr_in peerCopy = *manager->_peerAddress;

	// setup the channel
//	localCopy.rc_channel = 1;
//	peerCopy.rc_channel = 1;

	while ( manager->_setupThreadRunning )
	{
		manager->_currentState = State::Connecting;

		// clear the variables
		returnValue = 0;

		// get the socket
		localSocket = socket( AF_INET, SOCK_STREAM, 0 );

		// bind it
		returnValue |= bind( localSocket, (sockaddr*)&localCopy, sizeof( localCopy ) );

		// check for error
		if ( errno != 0 && returnValue <= 0 )
		{
			// there is an error, so close the socket and try again
			close( localSocket );
			continue;
		}

		// put the socket into listen mode
		returnValue |= listen( localSocket, 1 ) ;

		// accept the connection
		manager->_socket = accept( localSocket, (sockaddr*)&peerCopy, &opt  );

//		localCopy.rc_channel = peerCopy.rc_channel;

		// connected, so start sending and receiving data
		manager->startSendAndReceiveThreads();

		// close the socket
		close( localSocket );

		// set the state
		manager->_currentState = State::Connected;

		// wait for send and threads to join
		pthread_join( manager->_receiveThread, NULL );
		pthread_join( manager->_sendThread, NULL );

		// set the flags to false
		manager->_sendThreadRunning = false;
		manager->_receiveThreadRunning = false;

		// close the socket
		close( manager->_socket );
	}

	pthread_exit( NULL );
	return NULL;
}

void* Manager::handleCallbacks( void* input )
{
	// init the local variables
	Manager* manager = static_cast< Manager* >( input );
	Manager::State::Enum localState = manager->_currentState;
	uint32_t pollRate = manager->_pollRate;

	Manager::Interface* listenerObject = manager->_listenerObject;
	Manager::stateChange listenerMethod = manager->_listenerMethod;

	while ( true )
	{
		Manager::State::Enum tempState = manager->_currentState;
		if ( localState != tempState )
		{
			localState = tempState;
			// there is so signal the new state
			(listenerObject->*listenerMethod)( tempState );
		}

		// sleep for the receive/send rate
		usleep( pollRate );
	}

	pthread_exit( NULL );
	return NULL;
}

void Manager::startSendAndReceiveThreads()
{
	LibBBB::ThreadHelper::startJoinableThread( &_receiveThread, receiveMessage, &_receiveThreadRunning, static_cast< void* >( this ) );
	LibBBB::ThreadHelper::startJoinableThread( &_sendThread, sendMessage, &_sendThreadRunning, static_cast< void* >( this ) );
}
} // namespace Bluetooth
} // namespace LibBBB
