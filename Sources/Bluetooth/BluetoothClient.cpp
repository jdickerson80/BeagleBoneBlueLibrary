//#include "BluetoothClient.h"

//#include <bluetooth/bluetooth.h>
//#include <bluetooth/rfcomm.h>
//#include <unistd.h>
//#include "ThreadHelper.h"

//namespace VehicleControl {
//namespace Bluetooth {

//#define BluetoothClientPollPeriod ( 4 )

//Client::Client( const std::string& peerAddress )
//	: Base( peerAddress )
//	, _isConnected( false )
//{
//	Core::ThreadHelper::startDetachedThread( &this->_setupThread, tryToConnectToServer, &_setupThreadRunning, static_cast< void* >( this ) );
//}

//Client::~Client()
//{

//}

//bool Client::isConnected() const
//{
//	return _isConnected;
//}

//void Client::handleConnectionLoss()
//{
//	printf("client handled\r\n");
//	Core::ThreadHelper::startDetachedThread( &this->_setupThread, tryToConnectToServer, &_setupThreadRunning, static_cast< void* >( this ) );
//}

//void* Client::tryToConnectToServer( void* input )
//{
//	Client* client = static_cast< Client* >( input );

//	unsigned int returnValue = 0;

//	char buff[ 18 ] = { 0 };

//	client->_isConnected = false;

//	ba2str( &client->_peerAddress->rc_bdaddr, buff );
//	client->_peerAddress->rc_channel = 1;

//	printf("connecting with:\r\n"
//		   "address:\t%s\r\n"
//		   "channel:\t%d\r\n"
//		   "family :\t%d\r\n", buff, client->_peerAddress->rc_channel, client->_peerAddress->rc_family );

//	while ( true )
//	{
//		client->setupSocket();

//		returnValue |= connect( client->_socket, (sockaddr*)client->_peerAddress, sizeof( *client->_peerAddress ) );

//		int errsv = errno;
//		printf("BT client tried to connect with a status of: %s %i\r\n", strerror( errsv ), returnValue );

//		if ( errsv )
//		{
//			returnValue = 0;
//			sleep( BluetoothClientPollPeriod );
//		}
//		else
//		{
//			break;
//		}
//	}

//	if ( returnValue )
//	{
//		client->_isConnected = false;
//	}
//	else
//	{
//		client->startBothThreads();
//		client->_isConnected = true;
//	}

//	client->_setupThreadRunning = false;

//	pthread_exit( NULL );
//	return 0;
//}

//} // namespace Bluetooth
//} // namespace VehicleControl
