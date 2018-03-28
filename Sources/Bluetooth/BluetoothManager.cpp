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
				  , Interface* listenerObject
				  , stateChange listenerMethod
				  , uint32_t receiveMessageSize
				  , uint32_t sendMessageSize
				  , uint32_t pollRate )
	: _receiveThreadRunning( false )
	, _sendThreadRunning( false )
	, _setupThreadRunning( false )
	, _socket( 0 )
	, _pollRate( pollRate )
	, _receiveMessageSize( receiveMessageSize )
	, _sendMessageSize( sendMessageSize )
	, _currentState( State::LostConnection )
	, _listenerObject( listenerObject )
	, _listenerMethod( listenerMethod )
{
	_localAddress = new sockaddr_rc;
	_peerAddress = new sockaddr_rc;

	*_localAddress = { 0 };
	*_peerAddress = { 0 };

	// setup the bluetooth family
	_localAddress->rc_family = AF_BLUETOOTH;
	_peerAddress->rc_family = AF_BLUETOOTH;

	// convert the string to the addresses
	str2ba( localAddress.c_str(), &_localAddress->rc_bdaddr );
	str2ba( peerAddress.c_str(), &_peerAddress->rc_bdaddr );

	_receiveMessageBuffer = (uint8_t*)malloc( _receiveMessageSize * sizeof( uint8_t ) );
	_sendMessageBuffer = (uint8_t*)malloc( _sendMessageSize * sizeof( uint8_t ) );

	printf( "const size %i\n", _receiveMessageSize * sizeof( uint8_t ) );

	startSetupThread();
}

Manager::~Manager()
{
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

	close( _socket );
	delete _localAddress;
	delete _peerAddress;
	delete _receiveMessageBuffer;
	delete _sendMessageBuffer;
}

ssize_t Manager::sendData( const uint8_t * const data )
{
	if ( _currentState != State::Connected )
	{
		printf("send returned -1\n");
		return -1;
	}

	memcpy( (void*)_sendMessageBuffer, data, _sendMessageSize );
	return 1;
}

const uint8_t* const Manager::receiveData() const
{
	if ( _currentState != State::Connected )
	{
		printf("receive returned\n");
		return NULL;
	}

	return _receiveMessageBuffer;
}

bool Manager::isConnected() const
{
	return _currentState == State::Connected;
}

void* Manager::receiveMessage( void* input )
{
	Manager* manager = static_cast< Manager* >( input );
	uint32_t messageSize = manager->_receiveMessageSize;

	while ( manager->_receiveThreadRunning )
	{
		ssize_t result = recv( manager->_socket, (void*)manager->_receiveMessageBuffer, messageSize, 0 );

		printf("rec error %s size %i\n", strerror( errno ), result );

		if ( errno == ENOTCONN )
		{
			printf("rec: starting connecting thread %s\n", strerror( errno ) );
			memset( (void*)manager->_receiveMessageBuffer, 0, messageSize );
			manager->stopSendAndReceiveThreads();
			manager->startSetupThread();
			break;
		}

		usleep( manager->_pollRate );
	}

	pthread_exit( NULL );
	return NULL;
}

void* Manager::setupConnection( void* input )
{
	Manager* manager = static_cast< Manager* >( input );
	unsigned int opt = sizeof( *manager->_peerAddress );
	int returnValue = 0;
	char buf[ 1024 ] = {0};
	Interface* listenerObject = manager->_listenerObject;
	stateChange listenerMethod = manager->_listenerMethod;
	manager->_currentState = State::Connecting;
	(listenerObject->*listenerMethod)( manager->_currentState );
	int localSocket = 0;
	int err = 0;

	sockaddr_rc localCopy = *manager->_localAddress;
	sockaddr_rc peerCopy = *manager->_peerAddress;

	localCopy.rc_channel = 1;
	peerCopy.rc_channel = 1;

	ba2str( &localCopy.rc_bdaddr, buf );
	fprintf( stdout, "Server: Local add is %s chan is %u fam is %u\n", buf, localCopy.rc_channel, localCopy.rc_family );

	while ( manager->_setupThreadRunning )
	{
		returnValue = 0;
		manager->_socket = 0;
		localSocket = 0;
		err = 0;
		localSocket = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );

		returnValue |= bind( localSocket, (sockaddr*)&localCopy, sizeof( localCopy ) );

		err = errno;
		if ( err != 0 && returnValue <= 0 )
		{
			close( localSocket );
			printf("thread error is %s %i\n", strerror( err ), localSocket );
			sleep( 3 );
			continue;
		}

		//put socket into listen mode
		returnValue |= listen( localSocket, 1 ) ;

		manager->_socket = accept( localSocket, (sockaddr*)&peerCopy, &opt  );

		ba2str( &peerCopy.rc_bdaddr, buf );
		fprintf( stdout, "Server: Peer add is %s chan is %u fam is %u\n", buf, peerCopy.rc_channel, peerCopy.rc_family );

		close( localSocket );
		manager->startSendAndReceiveThreads();
		manager->_currentState = State::Connected;
		manager->_setupThreadRunning = false;
		(listenerObject->*listenerMethod)( manager->_currentState );
		pthread_exit( NULL );
	}
	return NULL;
}

void* Manager::sendMessage( void* input )
{
	Manager* manager = static_cast< Manager* >( input );
	uint32_t messageSize = manager->_sendMessageSize;

	while ( manager->_sendThreadRunning )
	{
		ssize_t result = send( manager->_socket, (void*)manager->_sendMessageBuffer, messageSize, 0 );

		//	int err = errno;

		printf("send error %s size %i\n", strerror( errno ), result );

		if ( result < 0 && errno != 0 )
		{
			printf("send: starting connecting thread\n");
			manager->stopSendAndReceiveThreads();
			manager->startSetupThread();
			break;
		}

		usleep( manager->_pollRate );
	}
	return NULL;
}

void Manager::startSetupThread()
{
	LibBBB::ThreadHelper::startDetachedThread( &_setupThread, setupConnection, &_setupThreadRunning, static_cast< void* >( this ) );
}

void Manager::startSendAndReceiveThreads()
{
	LibBBB::ThreadHelper::startDetachedThread( &_receiveThread, receiveMessage, &_receiveThreadRunning, static_cast< void* >( this ) );
	LibBBB::ThreadHelper::startDetachedThread( &_sendThread, sendMessage, &_sendThreadRunning, static_cast< void* >( this ) );
}

void Manager::stopSendAndReceiveThreads()
{
	_receiveThreadRunning = false;
	_sendThreadRunning = false;
}

} // namespace Bluetooth
} // namespace LibBBB
