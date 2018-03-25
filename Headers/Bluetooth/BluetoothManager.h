/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <string>

class sockaddr_rc;

namespace VehicleControl {
namespace Bluetooth {

/**
 * @brief	The BluetoothManager class is the base class for the bluetooth client and server.
 *			This class contains helper methods to start threads, client address,
 *			and the socket. This class will take the string version of the client
 *			address, and turn it into the bluetooth address used in RFCOMM.
 */
class Manager
{
public:

	struct State
	{
		enum Enum
		{
			Connected,
			Connecting,
			LostConnection,
			Error
		};
	};

	class Interface;

	typedef int (Interface::*stateChange)( State::Enum );

private:

	bool _serverThreadRunning;

	// FD for the client
	int _socket;

	// pthread instance
//	pthread_t _clientThread;
	pthread_t _serverThread;

	// pointer to the peer address
	sockaddr_rc* _localAddress;

	sockaddr_rc* _peerAddress;

	State::Enum _currentState;

	Interface* _listenerObject;

	stateChange _listenerMethod;

public:

	/**
	 * @brief Base constructor
	 * @param peerAddress address of the device that this code is connected.
	 */
	Manager( const std::string& peerAddress
			 , const std::string& localAddress
			 , Interface* listenerObject
			 , stateChange listenerMethod );

	~Manager();

	ssize_t sendData( const uint8_t* const data, uint8_t sizeOfMessage );

	ssize_t receiveData( uint8_t* const data, uint8_t sizeOfMessage );

	bool isConnected() const;


private:

	/**
	 * @brief writeMessage
	 * @param input
	 * @return
	 */
	static void* connectServer( void* input );

//	static void* connectClient( void* input );

};

} // namespace Bluetooth
} // namespace VehicleControl

#endif // BLUETOOTHMANAGER_H
