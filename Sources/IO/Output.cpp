#include "Output.h"

#include <unistd.h>

namespace Core {
namespace IO {

Output::Output( int number, const Setup& setup )
	: IOBase( number, Direction::Output )
	, _toggleNumber( setup.toggleNumber )
	, _togglePeriod( setup.togglePeriod )
{

}

Output::~Output()
{
	pthread_cancel( this->_thread );
}

int Output::setValue( IOBase::Value::Enum value )
{
	switch( value )
	{
	case Value::High:
		return IOBase::write( "value", "1");
		break;
	case Value::Low:
		return IOBase::write( "value", "0");
		break;
	}
	return -1;
}

bool Output::isToggling() const
{
	return this->_threadRunning;
}

int Output::toggleOutput()
{
	if ( ( bool )this->getValue() )
	{
		return this->setValue( Value::Low );
	}
	else
	{
		return this->setValue( Value::High );
	}
}

int Output::toggleOutput( int time )
{
	return this->toggleOutput( -1, time );
}

int Output::toggleOutput( int numberOfTimes, int time )
{
	this->_toggleNumber = numberOfTimes;
	this->_togglePeriod = time;
	this->_threadRunning = true;

	if( pthread_create( &this->_thread, NULL, &threadedToggle, static_cast< void* >( this ) ) )
	{
		perror("GPIO: Failed to create the toggle thread");
		this->_threadRunning = false;
		return -1;
	}

	return 0;
}

void Output::changeToggleTime( int time )
{
	this->_togglePeriod = time;
}

void Output::cancelToggle()
{
	this->_threadRunning = false;
}

void Output::streamOpen()
{
	_stream.open( ( IOBase::path() + "value" ).c_str() );
}

void Output::streamClose()
{
	_stream.close();
}

void Output::streamWrite( IOBase::Value::Enum value )
{
	_stream << value << std::flush;
}

void* Output::threadedToggle( void* value )
{
	Output* output = static_cast< Output* >( value );

	bool isHigh = (bool) output->getValue(); //find current value
	while( output->_threadRunning )
	{
		if (isHigh)
		{
			output->setValue( IOBase::Value::High );
		}
		else
		{
			output->setValue( IOBase::Value::Low );
		}

		usleep( output->_togglePeriod * 500 );
		isHigh = !isHigh;
		if( output->_toggleNumber > 0 )
		{
			output->_toggleNumber--;
		}
		if( output->_toggleNumber == 0 )
		{
			output->_threadRunning = false;
		}
	}
	return 0;
}

} // namespace IO
} // namespace Core
