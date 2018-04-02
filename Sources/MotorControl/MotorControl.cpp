#include "MotorControl.h"

#include <cmath>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "ManagerPRUs.h"
#include "ThreadHelper.h"

// Macros
#define PRU_SERVO_LOOP_INSTRUCTIONS	( 48 )	// instructions per PRU servo timer loop
#define PRU_FREQUENCY_IN_MHz ( 200 )
#define Microseconds ( 1000000.0f )

namespace LibBBB  {
namespace IO {

MotorControl::MotorControl( const Motor::Enum motorNumber
							, float frequency
							, const RangeType& dutyCyclePulseWidthRange )
	: _threadRunning( false )
	, _isRamping( false )
	, _motorNumber( motorNumber )
	, _frequency( frequency )
	, _numberOfLoops( 0 )
	, _currentPulseWidth( 0 )
	, _finalRampedPulseWidth( 0 )
	, _pruPointer( LibBBB ::ManagerPRUs::instance().sharedMemoryPointer() )
	, _rampRate( 0 )
	, _dutyCycleConverter( RangeType( 1, 1000 ), dutyCyclePulseWidthRange )
{
	_pruPointer[ _motorNumber ] = 0;
	calculateThreadSleepTime( frequency );
	ThreadHelper::startDetachedThread( &_thread, handleWaveform, &_threadRunning, static_cast< void* >( this ) );
}

MotorControl::~MotorControl()
{
	if ( _pruPointer != NULL )
	{
		_pruPointer[ _motorNumber ] = 0;
	}

	if ( this->_threadRunning )
	{
		pthread_cancel( this->_thread );
	}
}

void MotorControl::setDesiredDegree( int32_t /* degree */ )
{
	// do nothing implementation
}

void MotorControl::setDesiredDegree( int32_t /* degree */, uint32_t /* rampTime */ )
{
	// do nothing implementation
}

int32_t MotorControl::desiredDegree() const
{
	return -1;
}

void MotorControl::setPulseWidth( uint32_t pulseWidth )
{
	// TODO: is this necessary? should it be the callers job to set
	// it correctly?
	// check if the pulse width is possible, based on the frequency
//	if ( (uint32_t)( ( 1 / _frequency ) * Microseconds ) > pulseWidth )
//	{
//		printf("returned freq is %f wid is %u\n", _frequency, pulseWidth );
//		return;
//	}

	// clear the ramping flag
	_isRamping = false; // be wary...

	// width possible, so calculate the loops
	calculateLoops( pulseWidth );
}

void MotorControl::setPulseWidth( uint32_t finalPulseWidth, uint32_t rampTime )
{
	// get a local copy of the pulse width, so this method
	// is not called multiple times
	uint32_t currentPulseWidth = pulseWidth();

	// check if the desired pulse width is the current pulse width
	if ( finalPulseWidth == currentPulseWidth )
	{
		// it is, so ignore the call
		return;
	}

	// calculate the delta of the final pulse width - the current pulse width
	int32_t pulseDelta = finalPulseWidth - currentPulseWidth;

	/**
	 * calculate the number of loops to achieve the desired ramp
	 *
	 * The formula is: rampTime * 1000 <-- this is done because ramp time is in milliseconds, not micro
	 *				   --------------
	 *				          1
	 *                      -----      <-- this is the formula for the thread sleep time or period
	 *				          f
	 */
	uint32_t numberOfThreadLoops = ( rampTime * 1000 ) / _sleepTime;

	// if the ramp time is less than the frequency
	if ( numberOfThreadLoops == 0 )
	{
		// so just set the desired pulse width and leave the method
		setPulseWidth( finalPulseWidth );
		return;
	}

	// store a member copy of the final pulse width for checking
	// if the ramp is complete in the thread
	_finalRampedPulseWidth = finalPulseWidth;

	/**
	 * calculate the per thread run adder
	 *
	 * The formula is:     pulseDelta		<-- number of pulses for the ramp
	 *				   -------------------
	 *				   numberOfThreadLoops  <-- number of times the thread will run for the ramp
	 */
	_rampRate = pulseDelta / (int32_t)numberOfThreadLoops;

	/**
	 * This variable represents the pulse width that will not be achieved by the
	 * _rampRate * numberOfThreadLoops. This forces the first pulse width of the
	 * ramp to possibly be a different adder than the rest of the pulse widths.
	 *
	 * For example, the final pulse width is 1500, the number of loops is 10, and
	 * the _rampRate is 145. If the _rampRate is just added 10 times, the
	 * final pulse width would be 1450 NOT 1500. The remainderPulses variable
	 * IS the 50 that the final pulse would be missing. Therefore, the 50
	 * is added on the first ramped pulse so subsequent pulse additions
	 * will be even, making a perfect ramp to the final pulse width at the cost
	 * of the first pulse being a different adder than the rest.
	 */
	int32_t remainderPulses = finalPulseWidth - ( currentPulseWidth + ( numberOfThreadLoops * _rampRate ) );

	// FINALLY, set the first pulse width of the ramp
	setPulseWidth( currentPulseWidth + remainderPulses );

	// set the ramp flag. This has to be last
	// because the thread is running at a very high
	// rate for some motors, and it seems to cause the
	// thread issues to assign it earlier in the method
	_isRamping = true; // be wary...
}

uint32_t MotorControl::pulseWidth() const
{
	return _currentPulseWidth;
}

void MotorControl::setFrequency( float frequency )
{
	_frequency = frequency;
	calculateThreadSleepTime( frequency );
}

float MotorControl::frequency() const
{
	return _frequency;
}

bool MotorControl::isRamping() const
{
	return _isRamping;
}

void MotorControl::setDutyCycle( uint32_t dutyCycle )
{
	// check if 0% duty cycle is desired
	if ( dutyCycle == 0 )
	{
		// set the pulse width to 0
		setPulseWidth( 0 );
	}
	else // 0% duty is not desired
	{
		// calculate the pulse width for the given duty cycle
		setPulseWidth( _dutyCycleConverter.convertXtoY( dutyCycle ) );
	}
}

void MotorControl::setDutyCycle( uint32_t dutyCycle, uint32_t rampTime )
{
	// check if 0% duty cycle is desired
	if ( dutyCycle == 0 )
	{
		// set the pulse width to 0
		setPulseWidth( 0, rampTime );
	}
	else // 0% duty is not desired
	{
		// calculate the pulse width for the given duty cycle
		setPulseWidth( _dutyCycleConverter.convertXtoY( dutyCycle ), rampTime );
	}
}

uint32_t MotorControl::dutyCycle() const
{
	return _dutyCycleConverter.convertYtoX( pulseWidth() );
}

void* MotorControl::handleWaveform( void* objectPointer )
{
	// create local variables for the thread to use
	// cast the void* to a pointer to this class
	MotorControl* motorControl = static_cast< MotorControl* >( objectPointer );
	uint32_t* const pruPointer = motorControl->_pruPointer;

	// while the thread should run
	while ( motorControl->_threadRunning )
	{
		// check if the PRU is done. basically, a mutex for the PRU
		if ( pruPointer[ motorControl->_motorNumber ] == 0 )
		{
			// check if the servo is ramping
			if ( motorControl->_isRamping )
			{
				// get a local copy of the current pulse width
				uint32_t currentPulseWidth = motorControl->pulseWidth();

				// calculate the next pulse width
				int32_t nextPulseWidth = currentPulseWidth + motorControl->_rampRate;

				// if the next pulse width will finish the ramp, set the flag to false
				if ( nextPulseWidth == (int32_t)motorControl->_finalRampedPulseWidth )
				{
					motorControl->_isRamping = false;
				}

				// calculate the PRU loops necessary to achive the desired
				// pulse width
				motorControl->calculateLoops( nextPulseWidth );
			}

			// set the PRU to the number of loops desired
			pruPointer[ motorControl->_motorNumber ] = motorControl->_numberOfLoops;
		}

		// sleep for the period ( 1 / frequency )
		usleep( motorControl->_sleepTime );
	}

	// app is shutting down, so exit the thread
	pthread_exit( NULL );

	// return nothing of value
	return NULL;
}

void MotorControl::calculateThreadSleepTime( float frequency )
{
	// convert the frequency to time and convert it to microseconds
	_sleepTime = uint32_t( ( 1 / frequency ) * Microseconds );
	//	printf("sleep time is %i for %f freq\n", _sleepTime, frequency );
}

void MotorControl::calculateLoops( uint32_t pulseWidth )
{
	_currentPulseWidth = pulseWidth;
	_numberOfLoops = ( pulseWidth * PRU_FREQUENCY_IN_MHz ) / PRU_SERVO_LOOP_INSTRUCTIONS;
	//	printf("calc loop = %u\n", _numberOfLoops );
}

uint32_t MotorControl::calculatePulseWidthFromDutyCycle( uint32_t dutyCycle )
{
	return ( dutyCycle / _frequency ) * 1000;
}

} // namespace IO
} // namespace LibBBB
