//#include "BluetoothServer.h"

//#include <bluetooth/bluetooth.h>
//#include <bluetooth/rfcomm.h>
//#include <bluetooth/hci.h>
//#include <bluetooth/hci_lib.h>
//#include <unistd.h>
//#include "ThreadHelper.h"

//namespace VehicleControl {
//namespace Bluetooth {

//Server::Server( const std::string& peerAddress, const std::string& localAddress )
//	: Base( peerAddress )
//	, _isConnected( false )
//{
//	_localAddress = new sockaddr_rc;

//	*_localAddress = { 0 };

//	// setup the bluetooth family
//	_localAddress->rc_family = AF_BLUETOOTH;

//	// setup the port
//	_localAddress->rc_channel = 0;

//	// convert the string to the addresses
//	str2ba( localAddress.c_str(), &_localAddress->rc_bdaddr );

//	Core::ThreadHelper::startDetachedThread( &this->_setupThread, waitForConnectionsConnections, &_setupThreadRunning, static_cast< void* >( this ) );
////	printf("finished server const with local of %s\r\n", localAddress.c_str() );
//}

//Server::~Server()
//{
//	close( _client );
//	delete _localAddress;
//}

//bool Server::isConnected() const
//{
//	return _isConnected;
//}

//void Server::handleConnectionLoss()
//{
//	printf("server handled\r\n");
//	Core::ThreadHelper::startDetachedThread( &this->_setupThread, waitForConnectionsConnections, &_setupThreadRunning, static_cast< void* >( this ) );
//}

//void* Server::waitForConnectionsConnections( void* input )
//{
//	Server* server = static_cast< Server* >( input );
//	unsigned int opt = sizeof( *server->_peerAddress );
//	const size_t Size = 4;
//	uint8_t test[ Size ];
//	unsigned int returnValue = 0;
//	char buf[ 1024 ] = {0};

//	server->_isConnected = false;

//	server->setupSocket();

//	returnValue |= bind( server->_socket, (struct sockaddr *)server->_localAddress, sizeof( *server->_localAddress ) );
//	printf("Binding success %d\n", returnValue );

//	//put socket into listen mode
//	returnValue |= listen( server->_socket, 1 ) ;

//	printf("socket in listen mode %d\n", returnValue );

//	server->_client = accept( server->_socket, (struct sockaddr *)server->_peerAddress, &opt );

//	ba2str( &server->_peerAddress->rc_bdaddr, buf );
//	fprintf( stdout, "Connection accepted from %s %u\r\n", buf, sizeof( test ) );

//	for ( size_t i = 0; i < Size; ++i )
//	{
//		test[ i ] = i + 10;

////			usleep( 250000 );
//	}

//	size_t rec = 0;
//	size_t looper = 0;
//	while ( true )
//	{
//		send( server->_client, (void*)&test, sizeof( test ) / sizeof( test[ 0 ] ), 0 );
////		usleep( 250000 );
//		rec = recv( server->_client, test, sizeof( test ) / sizeof( test[ 0 ] ), 0 ) ;
//		printf("rec %u\n", rec );
//		for ( looper = 0; looper < rec; ++looper )
//		{
//			printf("t = %u\n", test[ looper ] );
//		}
//		sleep( 2 );

//	}
////	memset( buf, 0, sizeof( buf ) );

////	// read data from the client
////	int bytes_read = read(server->_client, buf, sizeof(buf));
////	if( bytes_read > 0 ) {
////		printf("received [%s]\n", buf);
////	}
////	else
////	{
////		printf("bad\n");
////	}

//	int errsv = errno;


//	if ( returnValue )
//	{
//		server->_isConnected = false;
//	}
//	else
//	{
//		server->startBothThreads();
//		server->_isConnected = true;
//	}

//	server->_setupThreadRunning = false;

//	printf("conn %d\n", server->isConnected() );

//	pthread_exit( NULL );
//	return 0;
//}

//} // namespace Bluetooth
//} // namespace VehicleControl
