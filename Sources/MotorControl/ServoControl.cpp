#include "ServoControl.h"

namespace LibBBB  {
namespace IO {

ServoControl::ServoControl( const MotorControl::Motor::Enum motorNumber
							, float frequency
							, const ServoControl::RangeType& degreeRange
							, const ServoControl::RangeType& pulseRange )
	: MotorControl( motorNumber, frequency, pulseRange )
	, _degreeConverter( degreeRange, pulseRange )
{

}

ServoControl::~ServoControl()
{

}

void ServoControl::setDesiredDegree( int32_t degree )
{
	MotorControl::setPulseWidth( (uint32_t)_degreeConverter.convertXtoY( degree ) );
}

void ServoControl::setDesiredDegree( int32_t degree, uint32_t rampTime )
{
	MotorControl::setPulseWidth( (uint32_t)_degreeConverter.convertXtoY( degree ), rampTime );
}

int32_t ServoControl::desiredDegree() const
{
	return _degreeConverter.convertYtoX( MotorControl::pulseWidth() );
}


} // namespace IO
} // namespace LibBBB
