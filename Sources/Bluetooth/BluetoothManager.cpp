#include "BluetoothManager.h"

#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "ThreadHelper.h"
#include "BluetoothInterface.h"

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
	if ( listenerObject )
	{
		LibBBB::ThreadHelper::startDetachedThread( &_callbackThread, handleCallbacks, &_callbackThreadRunning, static_cast< void* >( this ) );
	}

	// create the addresses
	_localAddress = new sockaddr_rc;
	_peerAddress = new sockaddr_rc;

	// clear the address
	*_localAddress = { 0 };
	*_peerAddress = { 0 };

	// setup the bluetooth family
	_localAddress->rc_family = AF_BLUETOOTH;
	_peerAddress->rc_family = AF_BLUETOOTH;

	// convert the string to the addresses
	str2ba( localAddress.c_str(), &_localAddress->rc_bdaddr );
	str2ba( peerAddress.c_str(), &_peerAddress->rc_bdaddr );

	// create the send and receive buffers
	_receiveMessageBuffer = (uint8_t*)malloc( _receiveMessageSize * sizeof( uint8_t ) );
	_sendMessageBuffer = (uint8_t*)malloc( _sendMessageSize * sizeof( uint8_t ) );

	// init the pthread and conditional variables
	_receiveMutex = PTHREAD_MUTEX_INITIALIZER;

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
	pthread_mutex_destroy( &_receiveMutex );
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
	uint8_t buffer[ messageSize ];// = malloc( messageSize * sizeof( uint8_t ) );
	//	pthread_mutex_t* mutex = &manager->_receiveMutex;

	while ( manager->_receiveThreadRunning )
	{
		// recieve the message
		ssize_t result = recv( manager->_socket, buffer, messageSize, 0 );

		manager->_receiveTime = clock();

		// check if the socket is still connected
		if ( errno == ENOTCONN || result != messageSize )
		{
			printf("rec broke\n" );
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
	int32_t maxReceiveDelta = ( ( (int)pollRate * 2 ) * 3 ) / 100;

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
		ssize_t result = send( manager->_socket, (void*)manager->_sendMessageBuffer, messageSize, 0 );

		// set the condition flag to false
		manager->_sendCondition = false;

		// unlock the mutex
		pthread_mutex_unlock( mutex );

		//		printf("send error %s size %i\n", strerror( errno ), result );

		// check for send errors
		if ( result < 0 && errno != 0 )
		{
//			manager->_currentState = State::Connecting;
			// there is an error so stop send and receive threads, and start
			// the setup thread
			printf("send broke\n");
			break;
		}

		int delta = clock() - manager->_receiveTime;
		if ( delta > maxReceiveDelta )
		{
			printf("rec timed out delt %i rate %i\n", delta, maxReceiveDelta );
			break;
		}
//		printf("del %i rate %i\n", delta, maxReceiveDelta );

//		printf("sent %i\n", result );

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
			// check if there is a listener object
//			if ( listenerObject )
			{
				// there is so signal the new state
				(listenerObject->*listenerMethod)( tempState );
			}
		}

		// sleep for the receive/send rate
		usleep( pollRate );
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
	//	memset( (void*)manager->_receiveMessageBuffer, 0, manager->_receiveMessageSize );
	//	memset( (void*)manager->_sendMessageBuffer, 0, manager->_sendMessageSize );

	// clear the variables
	int localSocket;

	// get a local copy of the addresses
	sockaddr_rc localCopy = *manager->_localAddress;
	sockaddr_rc peerCopy = *manager->_peerAddress;

	// setup the channel
	//	localCopy.rc_channel = 1;
	//	peerCopy.rc_channel = 1;

	while ( manager->_setupThreadRunning )
	{
		manager->_currentState = State::Connecting;

//		// check if there is a listener object
//		if ( manager->_listenerObject && notFirstRun )
//		{
//			// there is so signal the new state
//			(manager->_listenerObject->*manager->_listenerMethod)( manager->_currentState );
//		}

//		notFirstRun = true;

		// clear the variables
		returnValue = 0;

		// get the socket
		localSocket = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );

		// bind it
		returnValue |= bind( localSocket, (sockaddr*)&localCopy, sizeof( localCopy ) );

		// check for error
		if ( errno != 0 && returnValue <= 0 )
		{
			// there is an error, so close the socket and try again
			close( localSocket );
			printf("thread error is %s %i\n", strerror( errno ), localSocket );
			sleep( 1 );
			continue;
		}

		// put the socket into listen mode
		returnValue |= listen( localSocket, 1 ) ;

		// accept the connection
		manager->_socket = accept( localSocket, (sockaddr*)&peerCopy, &opt  );

		// close the socket
		close( localSocket );

		// start the send and receive threads, and
		// shut this thread down
		manager->startSendAndReceiveThreads();
		manager->_currentState = State::Connected;

//		// check if there is a listener object
//		if ( manager->_listenerObject )
//		{
//			// there is so signal the new state
//			(manager->_listenerObject->*manager->_listenerMethod)( manager->_currentState );
//		}

//		printf("waiting to join\n");
		pthread_join( manager->_receiveThread, NULL );
//		printf("rec join\n");
		pthread_join( manager->_sendThread, NULL );
//		printf("send join\n");
		close( manager->_socket );
		manager->_sendThreadRunning = false;
		manager->_receiveThreadRunning = false;
	}

	pthread_exit( NULL );
	return NULL;
}

void Manager::startSendAndReceiveThreads()
{
	_receiveTime = clock();
//	printf("start clock %li\n", _receiveTime );
	LibBBB::ThreadHelper::startJoinableThread( &_receiveThread, receiveMessage, &_receiveThreadRunning, static_cast< void* >( this ) );
	LibBBB::ThreadHelper::startJoinableThread( &_sendThread, sendMessage, &_sendThreadRunning, static_cast< void* >( this ) );
}
} // namespace Bluetooth
} // namespace LibBBB
