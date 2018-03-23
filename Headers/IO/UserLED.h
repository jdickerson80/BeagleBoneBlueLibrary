/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef USERLED_H_
#define USERLED_H_

#include <string>
#include <fstream>

namespace Core {
namespace IO {

/**
 * @brief	The UserLED class represents a user controlled LED. This class allows the
 *			LED to blink in a threaded manner. Additionally, the LED can be controlled
 *			in a "manual" way.
 * @todo	This class should inherit from IOBase, but Linux's LED control is significantly
 *			different than its GPIO control. However, IOBase can be refactored to accomodate
 *			the differences.
 */
class UserLED
{
public:

	/**
	 * @brief	The LED struct represents all of the BB Blue's available
	 *			user LEDs.
	 */
	struct LED
	{
		enum Enum
		{
			Green,
			Red,
			UserZero,
			UserOne,
			UserTwo,
			UserThree,
		};
	};

	/**
	 * @brief The State struct represents all of the states an LED can utilize.
	 */
	struct State
	{
		enum Enum
		{
			Blinking,
			On,
			Off
		};
	};

	/**
	 * @brief	The Setup struct represents the settings that an LED can have. This struct makes
	 *			it easier to create an LED because it allows the LED's constructor to have
	 *			a single arguement instead of an arguement for each setting.
	 */
	struct Setup
	{
		LED::Enum whatLED;
		int blinkNumber;
		int blinkPeriod;

		Setup( const LED::Enum whatLED )
			: whatLED( whatLED )
			, blinkNumber( 0 )
			, blinkPeriod( 0 )
		{}
	};

private:

	// flag to determine whether the thread is running
	bool _threadRunning;

	// path of the LED
	std::string _path;

	// pthread struct instance
	pthread_t thread;

	// number of times the LED blinks before shutting off
	int _blinkNumber;

	// period of the blinks
	int _blinkPeriod;

	// current state of the LED
	State::Enum _state;

public:

	/**
	 * @brief UserLED constructs an instance of a user led.
	 * @param setup
	 */
	UserLED( const Setup& setup );

	/**
	 * @brief ~UserLED destroys the LED.
	 */
	~UserLED();

	/**
	 * @brief setState method sets the state of the LED
	 * @param state new state of the LED
	 * @param numberOfBlinks before the blinking stops. -1 for infinity
	 * @param time duration of the blinking
	 * @return 0 for success, -1 for failure
	 */
	int setState( State::Enum state, int numberOfBlinks = 0, int time = 0 );

	/**
	 * @brief state getter
	 * @return current state of the LED
	 */
	UserLED::State::Enum state() const;

	/**
	 * @brief changeBlinkTime method changes the time of the blink
	 * @param time
	 */
	void changeBlinkTime( int time );

private:

	/**
	 * @brief handleBlinking method is called during threaded blinking
	 * @param value pointer to this class (this)
	 * @return NULL
	 */
	static void* handleBlinking( void* value );

	/**
	 * @brief	writeFile writes the value in the arguement to the
	 *			file in the _path variable.
	 * @param value to be written
	 * @return 0 for success, -1 for failure
	 */
	int writeFile( const std::string& value );
};

} // namespace IO
} // namespace Core

#endif // USERLED_H
