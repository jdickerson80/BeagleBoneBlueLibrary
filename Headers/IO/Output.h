/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "IOBase.h"

namespace LibBBB {
namespace IO {

/**
 * @brief	The Output class represents a GPIO pin that is set up as an output. This class has the
 *			ability to toggle outputs for a specific number of times in a threaded or non-threaded
 *			manner. Additionally, there are methods to "manually" control the output.
 *
 * @note	To construct an output, just create the setup struct, and pass the struct into the output's
 *			constructor. The setup struct, however, does not need to be kept in scope after it is passed
 *			to the constructor. This means that one could create the setup struct in the constructor
 *			of the output.
 */
class Output : public IOBase
{
public:

	/**
	 * @brief	The Setup struct represents the settings that an output can have. This struct makes
	 *			it easier to create an output because it allows the output's constructor to have
	 *			a single arguement instead of an arguement for each setting.
	 */
	struct Setup
	{
		int toggleNumber;
		int togglePeriod;

		Setup()
			: toggleNumber( 0 )
			, togglePeriod( 0 )
		{}
	};

private:

	// number of times the output toggles before quitting
	int _toggleNumber;

	// duration of toggling
	int _togglePeriod;

	// stream variable for fast output control
	std::ofstream _stream;

public:

	/**
	 * @brief	Output constructs an output based on the given setup. The constructor will automatically
	 *			export the pin, and do all the necessary Linux steps to create an output.
	 * @param setup
	 */
	Output( int number, const Setup& setup );

	/**
	 * @brief ~Output destroys the output. This automatically unexports the pin.
	 */
	~Output();

	/**
	 * @brief setValue setter
	 * @param value desired value of the output
	 * @return 0 for success, -1 for failure
	 */
	int setValue( Value::Enum value );

	/**
	 * @brief isToggling getter
	 * @return true if the output is toggling, false if it is not
	 */
	bool isToggling() const;

	/**
	 * @brief toggleOutput method toggles the output once
	 * @return 0 for success, -1 for failure
	 */
	int toggleOutput();

	/**
	 * @brief	toggleOutput method toggles the output every
	 *			using the time arguement.
	 * @param	time in milliseconds to invert the output.
	 * @return 0 for success, -1 for failure
	 */
	int toggleOutput( int time );

	/**
	 * @brief	toggleOutput method toggles the output for a given
	 *			numbers of times.
	 * @param numberOfTimes the output toggles before turning off
	 * @param time in milliseconds to invert the output.
	 * @return 0 for success, -1 for failure
	 */
	int toggleOutput( int numberOfTimes, int time );

	/**
	 * @brief changeToggleTime changes the toggling time
	 * @param time in milliseconds to invert the output.
	 * @note	this does not affect the toggling at all,
	 *			other than changing the time.
	 */
	void changeToggleTime( int time );

	/**
	 * @brief cancelToggle cancels toggling
	 */
	void cancelToggle();

private:

	/**
	 * @brief	streamOpen opens the output's file stream. This is used to
	 *			rapidly write to an output.
	 * @note this should be	avoided because it keeps the output's value file open.
	 */
	void streamOpen();

	/**
	 * @brief streamClose closes the output's file stream
	 * @note this should be	avoided because it keeps the output's value file open.
	 */
	void streamClose();

	/**
	 * @brief streamWrite writes to the output's file stream
	 * @param value to be written to the stream
	 * @note this should be	avoided because it keeps the output's value file open.
	 */
	void streamWrite( Value::Enum value );

	/**
	 * @brief threadedToggle method handles the toggling of the output in the thread
	 * @param value pointer to this class (this)
	 * @return NULL
	 */
	static void* threadedToggle( void* value );
};

} // namespace IO
} // namespace LibBBB

#endif // OUTPUT_H_
