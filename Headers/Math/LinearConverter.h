/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef LINEARCONVERTER_H
#define LINEARCONVERTER_H

#include "Range.h"

namespace LibBBB {
namespace Math {

/**
 * @brief	The LinearConverter class converts x to y or y to x using slope intercept formula.
 *			Basically, this class wraps mx + b, and allows conversion in the other direction.
 */
template< typename DataType >
class LinearConverter
{
public:

	// typedef of the range for convenience
	typedef Range< DataType > RangeType;

private:

	// x and y ranges
	const RangeType _xRange;
	const RangeType _yRange;

	// slope and y intercept
	DataType _denominatorSlope;
	DataType _numeratorSlope;
	DataType _b;

public:

	/**
	 * @brief LinearConverter constructor
	 * @param xValue range of the x value
	 * @param yValue range of the y value
	 */
	LinearConverter( const RangeType& xValue, const RangeType& yValue )
		: _xRange( xValue)
		, _yRange( yValue )
		, _denominatorSlope( xValue.range() )
		, _numeratorSlope( yValue.range() )
		, _b( -xValue.lowValue() * _numeratorSlope / _denominatorSlope + yValue.lowValue() )
	{}

	/**
	 * @brief convertXtoY getter. Uses y = mx + b.
	 * @param x value
	 * @return mx + b
	 */
	DataType convertXtoY( const DataType& x ) const
	{
		DataType returnValue = x * _numeratorSlope / _denominatorSlope + _b;
		return boundValue( returnValue, _yRange.lowValue(), _yRange.highValue() );
	}

	/**
	 * @brief convertYtoX getter. Uses x = ( y - b ) / m.
	 * @param y value
	 * @return ( y - b ) / m
	 */
	DataType convertYtoX( const DataType& y ) const
	{
		DataType returnValue = ( y - _b ) * _denominatorSlope / _numeratorSlope;
		return boundValue( returnValue, _xRange.lowValue(), _xRange.highValue() );
	}

private:

	/**
	 * @brief	boundValue method returns the bounded value based on the
	 *			given high and low values
	 * @param value to be bounded
	 * @param low lowest value "value" can be
	 * @param high highest value "value" can be
	 * @return bounded value
	 */
	inline DataType boundValue( const DataType& value, const DataType& low, const DataType& high ) const
	{
		if ( value < low )
		{
			return low;
		}

		if ( value > high )
		{
			return high;
		}

		return value;
	}
};

} // namespace Math
} // namespace LibBBB
#endif // LINEARCONVERTER_H
