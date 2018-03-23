/*
 * GPIO.cpp  Created on: 29 Apr 2014
 * Copyright (c) 2014 Derek Molloy (www.derekmolloy.ie)
 * Made available for the book "Exploring BeagleBone"
 * If you use this code in your work please cite:
 *   Derek Molloy, "Exploring BeagleBone: Tools and Techniques for Building
 *   with Embedded Linux", Wiley, 2014, ISBN:9781118935125.
 * See: www.exploringbeaglebone.com
 * Licensed under the EUPL V.1.1
 *
 * This Software is provided to You under the terms of the European
 * Union Public License (the "EUPL") version 1.1 as published by the
 * European Union. Any use of this Software, other than as authorized
 * under this License is strictly prohibited (to the extent such use
 * is covered by a right of the copyright holder of this Software).
 *
 * This Software is provided under the License on an "AS IS" basis and
 * without warranties of any kind concerning the Software, including
 * without limitation merchantability, fitness for a particular purpose,
 * absence of defects or errors, accuracy, and non-infringement of
 * intellectual property rights other than copyright. This disclaimer
 * of warranty is an essential part of the License and a condition for
 * the grant of any rights to this Software.
 *
 * For more details, see http://www.derekmolloy.ie/
 */

#include "IOBase.h"
#include <sstream>
#include <unistd.h>

#define GPIO_PATH "/sys/class/gpio/"

using namespace std;

namespace LibBBB  {
namespace IO {

IOBase::IOBase( int number, Direction::Enum whatDirection  )
	: _threadRunning( false )
	, _number( number )
{
	// set up the name and path variables
	ostringstream s;
	s << "gpio" << this->_number;
	this->_name = string(s.str());
	this->_path = GPIO_PATH + this->_name + "/";

	// call export on the pin to created it
	this->exportGPIO();

	// wait for Linux to set up the sysfs structure
	usleep( 250000 ); // 250ms delay

	// set the direction of the IO
	setDirection( whatDirection );
}

IOBase::~IOBase()
{
	this->unexportGPIO();
}

int IOBase::getNumber()
{
	return _number;
}

int IOBase::exportGPIO()
{
	stringstream s;
	s << this->_number;
	return this->writeWithPath( GPIO_PATH, "export", s.str() );
}

int IOBase::unexportGPIO()
{
	stringstream s;
	s << this->_number;
	return this->writeWithPath( GPIO_PATH, "unexport", s.str() );
}

int IOBase::setDirection( Direction::Enum dir)
{
	switch( dir )
	{
	case Direction::Input:
		return write( "direction", "in" );
		break;
	case Direction::Output:
		return write( "direction", "out" );
		break;
	}
	return -1;
}

int IOBase::setActiveHighOrLow( bool isLow )
{
	if ( isLow )
	{
		return write( "active_low", "1" );
	}
	else
	{
		return write( "active_low", "0" );
	}
}

int IOBase::write( const string& filename, const string& value )
{
	ofstream fs;
	fs.open( ( this->_path + filename ).c_str() );

	if ( !fs.is_open() )
	{
		printf("GPIO: write failed to open file %s", ( this->_path + filename ).c_str() );
		return -1;
	}

	fs << value;
	fs.close();
	return 0;
}

int IOBase::writeWithPath( const string& path, const string& filename, const string& value)
{
	ofstream fs;
	fs.open( ( path + filename ).c_str() );

	if ( !fs.is_open() )
	{
		printf("GPIO: write failed to open file %s", ( path + filename ).c_str() );
		return -1;
	}
	fs << value;
	fs.close();

	return 0;
}

int IOBase::write( const std::string& filename, int value )
{
	stringstream s;
	s << value;
	return write( filename, s.str() );
}

string IOBase::read( const std::string& filename ) const
{
	ifstream fs;
	fs.open( ( _path + filename ).c_str() );

	if ( !fs.is_open() )
	{
		printf("GPIO: read failed to open file %s", ( _path + filename ).c_str() );
	}

	string input;
	getline( fs,input );
	fs.close();

	return input;
}

const string& IOBase::name() const
{
	return _name;
}

const string& IOBase::path() const
{
	return _path;
}

IOBase::Value::Enum IOBase::getValue()
{
	string input = read( "value" );

	if ( input == "0" )
	{
		return Value::Low;
	}
	else
	{
		return Value::High;
	}
}

IOBase::Direction::Enum IOBase::getDirection()
{
	return _direction;
}

} // namespace IO
} // namespace LibBBB
