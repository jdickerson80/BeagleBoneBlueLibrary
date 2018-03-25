#include "BluetoothManager.h"

#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "ThreadHelper.h"
#include "BluetoothInterface.h"

namespace VehicleControl {
namespace Bluetooth {

Manager::Manager( const std::string& peerAddress
				 , const std::string& localAddress
				 , Interface* listenerObject
				 , stateChange listenerMethod )
	: _serverThreadRunning( false )
	, _socket( 0 )
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

	// setup the port
//	_localAddress->rc_channel = 2;
//	_peerAddress->rc_channel = 2;

	// convert the string to the addresses
	str2ba( localAddress.c_str(), &_localAddress->rc_bdaddr );
	str2ba( peerAddress.c_str(), &_peerAddress->rc_bdaddr );


//	char buf[ 1024 ] = {0};
//	ba2str( &_localAddress->rc_bdaddr, buf );
//	fprintf( stdout, "Const: Local add is %s chan is %u fam is %u\n", buf, _localAddress->rc_channel, _localAddress->rc_family );

//	ba2str( &_peerAddress->rc_bdaddr, buf );
//	fprintf( stdout, "Const: Peer add is %s chan is %u fam is %u\n", buf, _peerAddress->rc_channel, _peerAddress->rc_family );

//	Core::ThreadHelper::startDetachedThread( &_clientThread, connectClient, &_clientThreadRunning, static_cast< void* >( this ) );
	LibBBB::ThreadHelper::startDetachedThread( &_serverThread, connectServer, &_serverThreadRunning, static_cast< void* >( this ) );

//	printf("based finished const peer add is %s\r\n", peerAddress.c_str() );
}

Manager::~Manager()
{
	if ( this->_serverThreadRunning )
	{
		pthread_cancel( this->_serverThread );
	}

	close( _socket );
	delete _localAddress;
	delete _peerAddress;
}

ssize_t Manager::sendData( const uint8_t * const data, uint8_t sizeOfMessage )
{
	if ( _currentState != State::Connected )
	{
		printf("send returned -1\n");
		return -1;
	}

	ssize_t result = send( _socket, (void*)data, sizeOfMessage, MSG_DONTWAIT );

//	int err = errno;

//	printf("send error %s\n", strerror( err ) );

	if ( result < 0 && errno != 0 )
	{
		printf("send: starting connecting thread\n");
		LibBBB::ThreadHelper::startDetachedThread( &_serverThread, connectServer, &_serverThreadRunning, static_cast< void* >( this ) );
	}

	return result;
}

ssize_t Manager::receiveData( uint8_t* const data, uint8_t sizeOfMessage )
{
	if ( _currentState != State::Connected )
	{
		printf("receive returned -1\n");
		return -1;
	}

	ssize_t result = recv( _socket, (void*)data, sizeOfMessage, MSG_DONTWAIT );

	if ( result < 0 && errno == ENOTCONN )
	{
		printf("rec: starting connecting thread %s\n", strerror( errno ) );
		memset( (void*)data, 0, sizeOfMessage );
		LibBBB::ThreadHelper::startDetachedThread( &_serverThread, connectServer, &_serverThreadRunning, static_cast< void* >( this ) );
	}

	return result;
}

bool Manager::isConnected() const
{
	return _currentState == State::Connected;
}

void* Manager::connectServer( void* input )
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
//	printf("connecting thread\n");

	ba2str( &localCopy.rc_bdaddr, buf );
	fprintf( stdout, "Server: Local add is %s chan is %u fam is %u\n", buf, localCopy.rc_channel, localCopy.rc_family );

//	ba2str( &manager->_peerAddress->rc_bdaddr, buf );
//	fprintf( stdout, "Peer add is %s chan is %u fam is %u\n", buf, manager->_peerAddress->rc_channel, manager->_peerAddress->rc_family );

	while ( manager->_serverThreadRunning )
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

//		printf("Binding success %d\n", returnValue );

		//put socket into listen mode
		returnValue |= listen( localSocket, 1 ) ;

//		printf("socket in listen mode %d\n", returnValue );

		manager->_socket = accept( localSocket, (sockaddr*)&peerCopy, &opt  );

		ba2str( &peerCopy.rc_bdaddr, buf );
		fprintf( stdout, "Server: Peer add is %s chan is %u fam is %u\n", buf, peerCopy.rc_channel, peerCopy.rc_family );

//		ba2str( &peerCopy.rc_bdaddr, buf );
		fprintf( stdout, "Server: Connection accepted from %s\r\n", buf );


		close( localSocket );
		manager->_currentState = State::Connected;
		manager->_serverThreadRunning = false;
		(listenerObject->*listenerMethod)( manager->_currentState );
		pthread_exit( NULL );
	}
	return NULL;
}

} // namespace Bluetooth
} // namespace VehicleControl
