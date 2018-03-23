#include "UserLED.h"

#include <unistd.h>
#include <sstream>
#include "ThreadHelper.h"

#define PathBase  ( "/sys/class/leds" )
#define GreenPath ( "/green/" )
#define RedPath ( "/red/" )

#define User0Path ( "/beaglebone:green:usr0/" )
#define User1Path ( "/beaglebone:green:usr1/" )
#define User2Path ( "/beaglebone:green:usr2/" )
#define User3Path ( "/beaglebone:green:usr3/" )

namespace LibBBB  {
namespace IO {

std::string enumToString( UserLED::LED::Enum whatLED )
{
	std::ostringstream stream;
	stream << PathBase;

	switch ( whatLED )
	{
	case UserLED::LED::Green:
		stream << GreenPath;
		break;

	case UserLED::LED::Red:
		stream << RedPath;
		break;

	case UserLED::LED::UserZero:
		stream << User0Path;
		break;

	case UserLED::LED::UserOne:
		stream << User1Path;
		break;

	case UserLED::LED::UserTwo:
		stream << User2Path;
		break;

	case UserLED::LED::UserThree:
		stream << User3Path;
		break;
	}

	return stream.str();
}

UserLED::UserLED( const Setup& setup )
	: _threadRunning( false )
	, _blinkNumber( setup.blinkNumber )
	, _blinkPeriod( setup.blinkPeriod )
	, _state( State::Off )
{
//	this->_name = string(s.str());
	this->_path = enumToString( setup.whatLED );

	// need to give Linux time to set up the sysfs structure
	usleep( 250000 ); // 250ms delay
}

UserLED::~UserLED()
{
	writeFile( "0" );

	if ( this->_threadRunning )
	{
		pthread_cancel( this->thread );
	}
}

int UserLED::writeFile( const std::string& value )
{
	std::ofstream fs;
	fs.open( ( this->_path + "brightness" ).c_str() );
	if ( !fs.is_open() )
	{
		printf("LED: write failed to open file %s", ( this->_path ).c_str() );
		return -1;
	}
	fs << value;
	fs.close();
	return 0;
}

int UserLED::setState( State::Enum state, int numberOfBlinks /* = 0 */, int time /* = 0 */ )
{
	if ( state == _state )
	{
		return 0;
	}

	_state = state;

	int returnValue = 0;

	switch( _state )
	{
	case State::Blinking:
		this->_blinkNumber = numberOfBlinks;
		this->_blinkPeriod = time;

		returnValue |= ThreadHelper::startDetachedThread(
					&this->thread
					, &handleBlinking
					, &this->_threadRunning
					, static_cast< void* >( this ) );

		if( returnValue )
		{
			perror("GPIO: Failed to create the poll thread");
			this->_threadRunning = false;
			return -1;
		}
		else
		{
			return 0;
		}

		break;

	case State::Off:
		this->_threadRunning = false;
		return writeFile( "0" );
		break;

	case State::On:
		this->_threadRunning = false;
		return writeFile( "1" );
		break;
	}

	return -1;
}

UserLED::State::Enum UserLED::state() const
{
	return _state;
}

void UserLED::changeBlinkTime( int time )
{
	this->_blinkPeriod = time;
}

void* UserLED::handleBlinking( void* value )
{
	UserLED* led = static_cast< UserLED* >( value );

	bool isOn = true;
	std::string ledOutput = "1";
	led->writeFile( ledOutput );

	while( led->_threadRunning )
	{
		if( led->_blinkNumber > 0 )
		{
			led->_blinkNumber--;
		}

		if( led->_blinkNumber == 0 )
		{
			led->_threadRunning = false;
			led->setState( State::Off );
			break;
		}

		isOn = !isOn;

		ledOutput = isOn == true
				? "1"
				: "0";

		led->writeFile( ledOutput );

		usleep( led->_blinkPeriod * 500 );
	}

	pthread_exit( NULL );
	return 0;
}

} // namespace IO
} // namespace LibBBB
