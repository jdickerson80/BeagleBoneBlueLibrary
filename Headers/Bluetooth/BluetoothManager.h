/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <string>

class sockaddr_rc;

namespace LibBBB {
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

	bool _receiveThreadRunning;
	bool _sendThreadRunning;
	bool _setupThreadRunning;

	// FD for the client
	int _socket;

	// pthread instances
	pthread_t _receiveThread;
	pthread_t _sendThread;
	pthread_t _setupThread;

	uint32_t _pollRate;
	uint32_t _receiveMessageSize;
	uint32_t _sendMessageSize;

	uint8_t* _receiveMessageBuffer;
	uint8_t* _sendMessageBuffer;

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
			 , stateChange listenerMethod
			 , uint32_t receiveMessageSize
			 , uint32_t sendMessageSize
			 , uint32_t pollRate );

	~Manager();

	ssize_t sendData( const uint8_t* const data );

	const uint8_t* const receiveData() const;

	bool isConnected() const;


private:

	static void* receiveMessage( void* input );
	/**
	 * @brief writeMessage
	 * @param input
	 * @return
	 */
	static void* setupConnection( void* input );

	static void* sendMessage( void* input );

	void startSetupThread();

	void startSendAndReceiveThreads();

	void stopSendAndReceiveThreads();

//	static void* connectClient( void* input );

};

} // namespace Bluetooth
} // namespace LibBBB

#endif // BLUETOOTHMANAGER_H
