/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <stdint.h>
#include <pthread.h>
#include "LinearConverter.h"

namespace LibBBB {
namespace IO {

/**
 * @brief	The MotorControl class controls a PRU controlled servo. It handles
 *			all control of the servo via this class's API. This class spawns a thread
 *			to better independently control the frequency of the signal. This allows
 *			each motor to have a unique frequency to better control the different
 *			types of servos.
 * @todo	Add ranges to this class to allow easy control of hobbyist servos that
 *			are more convenient to use a degree range, not a pulse width range.
 */
class MotorControl
{
public:

	// typedef the appropriate range for the class
	typedef Math::Range< int32_t > RangeType;

	/**
	 * @brief	The Servo struct holds all of the allowable motor channels.
	 * @note	This class also accounts for the motor's being zero based
	 *			programmatically, but 1 based in the "real" world.
	 */
	struct Motor
	{
		enum Enum
		{
			// Zero based offset
			One = 0, Two, Three, Four,
			Five, Six, Seven, Eight
		};
	};

protected:

	// typedef the converter for easier instantiation
	typedef Math::LinearConverter< int32_t > Converter;

private:

	// Member variables
	bool _threadRunning;
	bool _isRamping;
	const Motor::Enum _motorNumber;
	float _frequency;
	uint32_t _numberOfLoops;
	uint32_t _sleepTime;
	uint32_t _currentPulseWidth;
	uint32_t _finalRampedPulseWidth;
	uint32_t* const _pruPointer;
	int32_t _rampRate;
	pthread_t _thread;
	Converter _dutyCycleConverter;

public:

	/**
	 * @brief	MotorControl ServoControl constructor creates the class, and spawns the
	 *			frequency control thread.
	 * @param	motorNumber the pin of the motor
	 * @param	frequency of the motor
	 * @param	dutyCyclePulseWidthRange the minimum and maximum pulse width used
	 *			in the duty cycle calculation.
	 */
	MotorControl( const Motor::Enum motorNumber
				  , float frequency
				  , const RangeType& dutyCyclePulseWidthRange );

	/**
	 * @brief	ServoMotorControl destructor stops the thread
	 */
	virtual ~MotorControl();

	/**
	 * @brief setDesiredDegree method sets the desired servo position in degrees
	 * @param degree position in degrees
	 * @note	This method does nothing in this class. It is just here for polymorphic reason.
	 *			If these methods are desired, use the ServoControl class
	 */
	virtual void setDesiredDegree( int32_t degree );

	/**
	 * @brief setDesiredDegree method sets the desired servo position in degrees
	 * @param degree position in degrees
	 * @param rampTime in milliseconds
	 * @note	This method does nothing in this class. It is just here for polymorphic reason.
	 *			If these methods are desired, use the ServoControl class
	 */
	virtual void setDesiredDegree( int32_t degree, uint32_t rampTime );

	/**
	 * @brief desiredDegree getter
	 * @return ALWAYS -1
	 * @note	This method does nothing in this class. It is just here for polymorphic reason.
	 *			If these methods are desired, use the ServoControl class
	 */
	virtual int32_t desiredDegree() const;

	/**
	 * @brief pulseWidth getter
	 * @return pulse width in microseconds
	 */
	uint32_t pulseWidth() const;

	/**
	 * @brief setPulseWidth sets the pulse width of the signal
	 * @param pulseWidth width of the pulse in microsecond
	 * @note if this method is called, it will cancel ramping functionality
	 */
	void setPulseWidth( uint32_t pulseWidth );

	/**
	 * @brief setPulseWidth sets the final pulse width of the signal using the given ramp time
	 * @param pulseWidth width of the pulse width in microsecond
	 * @param rampTime time, in milliseconds, to achieve the final pulse width
	 */
	void setPulseWidth( uint32_t finalPulseWidth, uint32_t rampTime );

	/**
	 * @brief setFrequency sets the frequency of the signal
	 * @param frequency in hz
	 */
	void setFrequency( float frequency );

	/**
	 * @brief frequency getter
	 * @return freqency in hertz
	 */
	float frequency() const;

	/**
	 * @brief isRamping getter
	 * @return true if the servo is ramping, false if it is not
	 */
	bool isRamping() const;

	/**
	 * @brief setDutyCycle setter
	 * @param dutyCycle in percent * 10, so 100% is 1000
	 */
	void setDutyCycle( uint32_t dutyCycle );

	/**
	 * @brief setDutyCycle
	 * @param dutyCycle
	 * @param rampTime
	 */
	void setDutyCycle( uint32_t dutyCycle, uint32_t rampTime );

	/**
	 * @brief dutyCycle getter
	 * @return current duty cycle
	 */
	uint32_t dutyCycle() const;

private:

	/**
	 * @brief handleWaveform handles the waveform of the signal
	 * @param objectPointer pointer to an instance of this class
	 * @return NULL
	 */
	static void* handleWaveform( void* objectPointer );

	/**
	 * @brief calculateThreadSleepTime calculates the thread sleep time given
	 *			the frequency.
	 * @param frequency in hz
	 */
	void calculateThreadSleepTime( float frequency );

	/**
	 * @brief calculateLoops calculates the number of loops given the pulse width
	 * @param pulseWidth in microsecons
	 * @return number of PRU loops
	 */
	void calculateLoops( uint32_t pulseWidth );

	/**
	 * @brief calculatePulseWidthFromDutyCycle calculates the pulse
	 *		  width for a given duty cycle.
	 * @param dutyCycle to be calculated. Duty cycles is in the scale of
	 *		  100.0%, so 100% is 1000;
	 * @return pulse width
	 */
	uint32_t calculatePulseWidthFromDutyCycle( uint32_t dutyCycle );
};

} // namespace IO
} // namespace LibBBB
#endif // MOTORCONTROL_H
