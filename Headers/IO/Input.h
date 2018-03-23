/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef INPUT_H_
#define INPUT_H_

#include "IOBase.h"

namespace LibBBB {
namespace IO {

/**
 * @brief	The Input class represents a GPIO pin that is set up as an input. This class allows the user to
 *			add a callback function to either the rising, falling, or both edges of the input. This means
 *			that the function will automatically be called when its respective edge event happens.
 *			There are two different ways to use the edge callback:
 *				1) waitForEdge() - where it blocks until the edge event happens.
 *				2) waitForEdgeThreaded - which does the same thing, without blocking.
 *			If edge events are undesired, there are standard methods to just query the state of the input.
 *
 * @note	To construct an input, just create the setup struct, and pass the struct into the inputs's
 *			constructor. The setup struct, however, does not need to be kept in scope after it is passed
 *			to the constructor. This means that one could create the setup struct in the constructor
 *			of the input.
 *
 * @note	Upon construction, if there is a callback function in the Setup structure,
 *			the constructor will automatically call the threaded wait for edge.
 */
class Input : public IOBase
{
public:

	/**
	 * Typedef of the callback function
	 */
	typedef int (*CallbackType)( int );


	/**
	 * @brief The Edge struct holds the enum that represents the edge events.
	 */
	struct Edge
	{
		enum Enum
		{
			None	= 0,
			Rising,
			Falling,
			Both
		};
	};

	/**
	 * @brief	The Setup struct represents the settings that an input can have. This struct makes
	 *			it easier to create an input because it allows the input's constructor to have
	 *			a single arguement instead of an arguement for each setting.
	 */
	struct Setup
	{
		bool isActiveLow;
		int debounceTime;
		Input::Edge::Enum desiredEdge;
		CallbackType callback;

		Setup()
		: isActiveLow( false )
		, debounceTime( 0 )
		, desiredEdge( Edge::None )
		, callback( NULL )
		{}
	};

private:

	// local copy of debounce time
	int _debounceTime;

	// callback function
	CallbackType _callbackFunction;

public:

	/**
	 * @brief	Input constructs an input based on the given setup. The constructor will automatically
	 *			export the pin, and do all the necessary Linux steps to create an input.
	 * @param setup
	 */
	Input( int number, const Setup& setup );

	/**
	 * @brief ~Input destroys the input. This automatically unexports the pin.
	 */
	~Input();

	/**
	 * @brief	setDebounceTime sets the debounce time of the input. This ensures the input
	 *			is in the proper state for the debounce time before the thread calls its respective
	 *			callback function.
	 * @param	time in milliseconds
	 * @note http://whatis.techtarget.com/definition/debouncing
	 */
	void setDebounceTime( int time );

	/**
	 * @brief getDebounceTime getter
	 * @return time in milliseconds
	 */
	int getDebounceTime() const;

	/**
	 * @brief setEdgeType sets the desired edge
	 * @param edge desired event edge
	 * @return 0 for success, -1 for failure
	 */
	int setEdgeType( Input::Edge::Enum edge );

	/**
	 * @brief getEdgeType getter
	 * @return desired event edge
	 */
	Input::Edge::Enum getEdgeType();

	/**
	 * @brief setEdgeCallback setter
	 * @param callback is a callback function that is to be called when the correct event has occured
	 */
	void setEdgeCallback( CallbackType callback = NULL );

	/**
	 * @brief	waitForEdge method is a blocking wait for edge. Therefore, the callee will halt its thread
	 *			until this method has returned.
	 * @return	0 for success, and -1 for failure
	 */
	int waitForEdge();

	/**
	 * @brief	waitForEdgeThreaded does the same thing as waitForEdge(), but does so in a threaded manner.
	 *			This allows the calling program to continue execution after this methods has been called.
	 * @return	0 for success, and -1 for failure
	 */
	int waitForEdgeThreaded();

	/**
	 * @brief	cancelWaitForEdge method cancels waiting for the edge.
	 * @note	This exits the underlying thread
	 */
	void cancelWaitForEdge();

private:

	/**
	 * @brief	threadedPoll method is the method that is called whenever waiting on edge occurs.
	 *			This method just loops continuously until the thread running variable goes false
	 * @param value pointer to this class (this)
	 * @return NULL
	 */
	static void* threadedPoll( void* value );
};

} // namespace IO
} // namespace LibBBB

#endif // INPUT_H_
