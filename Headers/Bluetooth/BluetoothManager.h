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
 * @brief	The BluetoothManager class manages the Bluetooth connection. It
 *			puts the socket into listen mode to connect to the remote device,
 *			and creates a read and write thread to handle sending and receiving
 *			data across the Bluetooth connection. This class also has a mutex
 *			and conditional variable to handle the race condition of the send
 *			buffer.
 */
class Manager
{
public:

	/**
	 * @brief	The State struct represents the state of the Bluetooth
	 *			connection.
	 */
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

	// forward declaration of the interface class
	class Interface;

	// typedef of the prototype of the state change method
	typedef int (Interface::*stateChange)( State::Enum );

private:

	// flags for the threads running state
	bool _receiveThreadRunning;
	bool _sendCondition;
	bool _sendThreadRunning;
	bool _setupThreadRunning;

	// FD for the client
	int _socket;

	// pthread instances
	pthread_t _receiveThread;
	pthread_t _sendThread;
	pthread_t _setupThread;

	// mutex and conditional variable
	pthread_cond_t _conditionalVariable;
	pthread_mutex_t _mutex;

	// constants for sending and receiving
	const uint32_t _pollRate;
	const uint32_t _receiveMessageSize;
	const uint32_t _sendMessageSize;

	// send and receive buffers
	uint8_t* _receiveMessageBuffer;
	uint8_t* _sendMessageBuffer;

	// pointer to the peer address
	sockaddr_rc* _localAddress;
	sockaddr_rc* _peerAddress;

	// state of the connection
	State::Enum _currentState;

	// local variables for the callbacks
	Interface* _listenerObject;
	stateChange _listenerMethod;

public:

	/**
	 * @brief Manager creates an instance of the Bluetooth manager
	 * @param peerAddress address of the remote device
	 * @param localAddress address of this device
	 * @param receiveMessageSize size of the receive message
	 * @param sendMessageSize size of the send message
	 * @param pollRate send and receive rate
	 * @param listenerObject pointer to the interface
	 * @param listenerMethod pointer to the interface's method
	 */
	Manager( const std::string& peerAddress
			 , const std::string& localAddress
			 , uint32_t receiveMessageSize
			 , uint32_t sendMessageSize
			 , uint32_t pollRate
			 , Interface* listenerObject = NULL
			 , stateChange listenerMethod = NULL );

	/**
	 * @brief ~Manager do nothing destructor
	 */
	~Manager();

	/**
	 * @brief sendData method that sends the data over the Bluetooth connection
	 * @param data to be sent
	 * @return ??
	 */
	ssize_t sendData( const uint8_t* const data );

	/**
	 * @brief receiveData method that receives the data
	 * @return	pointer to the data. If there is no new data or the connection
	 *			is down, NULL is returned.
	 */
	const uint8_t* const receiveData() const;

	/**
	 * @brief isConnected getter
	 * @return true if connected, false if it is not
	 */
	bool isConnected() const;

private:

	/**
	 * @brief	receiveMessage thread that handles receiving the data. This
	 *			is a thread so Linux's recv function can be blocking. This
	 *			helps ensure the whole message is received all at once, instead
	 *			of having to cache parts of the message, and waiting until the
	 *			next recv call to concatenate the full message from the newly
	 *			received data, and the cached data.
	 * @param input pointer to this class
	 * @return NULL
	 */
	static void* receiveMessage( void* input );

	/**
	 * @brief	setupConnection thread handles listening for the peer connection.
	 *			This method binds the socket and puts it in listening mode. Since
	 *			this is a thread, it will block until it connects to the peer device.
	 *			Once the device connects, it will exit this thread and start the send
	 *			and receive threads.
	 * @param input pointer to this class
	 * @return NULL
	 */
	static void* setupConnection( void* input );

	/**
	 * @brief	writeMessage thread that handles sending the data. This
	 *			is a thread so Linux's send function can be blocking. This
	 *			helps ensure the whole message is sent all at once, instead
	 *			of having to cache parts of the message, and waiting until the
	 *			next send call to send the rest of the message.
	 * @param input pointer to this class
	 * @return NULL
	 */
	static void* sendMessage( void* input );

	/**
	 * @brief	startSetupThread method just wraps starting the setup thread.
	 * @note	this is purely a convenience method
	 */
	void startSetupThread();

	/**
	 * @brief	startSendAndReceiveThreads method starts the send and receive
	 *			threads
	 * @note	this is purely a convenience method
	 */
	void startSendAndReceiveThreads();

	/**
	 * @brief	stopSendAndReceiveThreads stops the send and receive threads.
	 *			This is used, for instance, when the peer device has been disconnected.
	 * @note	this is purely a convenience method
	 */
	void stopSendAndReceiveThreads();
};

} // namespace Bluetooth
} // namespace LibBBB

#endif // BLUETOOTHMANAGER_H
