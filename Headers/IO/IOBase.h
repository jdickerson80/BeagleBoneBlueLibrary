/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef IOBASE_H_
#define IOBASE_H_

#include <string>
#include <fstream>

namespace Core {
namespace IO {

/**
 * @brief	The IOBase class is the base class for all GPIO. This class contains all of the methods to call into and
 *			manipulate Linux's GPIO file system, /sys/class/gpio. This class also calls export and unexport
 *			to create and remove GPIO pins.
 */
class IOBase
{
public:

	/**
	 * @brief The Direction struct holds the enum to represent an Input or Output.
	 */
	struct Direction
	{
		enum Enum
		{
			Input,
			Output
		};
	};

	/**
	 * @brief The Value struct holds the enyn to represent the GPIO being low or high.
	 */
	struct Value
	{
		enum Enum
		{
			High = 1,
			Low = 0
		};
	};

public:

	/**
	 * @brief ~IOBase unexports the GPIO pin
	 */
	virtual ~IOBase();

	/**
	 * @brief getNumber getter
	 * @return pin number
	 */
	int getNumber();

	/**
	 * @brief getValue getter
	 * @return whether the pin is either high or low
	 */
	Value::Enum getValue();

	/**
	 * @brief getDirection getter
	 * @return whether the pin is an input or an output
	 */
	Direction::Enum getDirection();

protected:

	/**
	 * @brief IOBase creates an instance of the base io class
	 * @param number what pin
	 * @param whatDirection input or output
	 */
	IOBase( int number, Direction::Enum whatDirection );

	/**
	 * @brief setDirection setter
	 * @param direction determines whether this IO instance is an input or output
	 * @return
	 */
	int setDirection( Direction::Enum direction );

	/**
	 * @brief setActiveHighOrLow setter
	 * @param	isLow (default true) determines what state the IO is in during
	 *			it untriggered state.
	 *			1) For Inputs:
	 *				true  - the input will be off while it is not triggered
	 *				false - the input will be on while it is not triggered
	 * 			2) For Outputs:
	 *				true  - TODO: finish
	 *				false - TODO: finish
	 * @return 0 for success, -1 for failure
	 */
	int setActiveHighOrLow( bool isLow = true );  //low=1, high=0 !!!!!!!!!!!!!11

	/**
	 * @brief writeWithPath methods writes the value into the given path and filename
	 * @param path The path of the file to be modified
	 * @param filename the file to be written to in that path
	 * @param value to be written to the file
	 * @return 0 for success, -1 for failure
	 */
	static int writeWithPath( const std::string& path, const std::string& filename, const std::string& value );

	/**
	 * @brief write write function that writes a single string value to a file in the path provided
	 * @param filename the file to be written to in that path
	 * @param value value to be written to the file
	 * @return 0 for success, -1 for failure
	 */
	int write( const std::string& filename, const std::string& value );

	/**
	 * @brief write write function that writes a single string value to a file in the path provided
	 * @param filename the file to be written to in that path
	 * @param value value to be written to the file
	 * @return 0 for success, -1 for failure
	 */
	int write( const std::string& filename, int value );

	/**
	 * @brief read function that reads the value of the given file filename
	 * @param filename the file to be written to in that path
	 * @return value in the given file
	 */
	std::string read( const std::string& filename ) const;

	/**
	 * @brief name getter
	 * @return name of the associated file
	 */
	const std::string& name() const;

	/**
	 * @brief path getter
	 * @return path of the associated file
	 */
	const std::string& path() const;

protected:

	// whether the thread should run
	bool _threadRunning;

	// instance of the pthread struct
	pthread_t _thread;

private:

	// name of the IO
	std::string _name;

	// path of the IO
	std::string _path;

	// number of the io pin
	int _number;

	// whether this is an input or output
	Direction::Enum _direction;

private:

	/**
	 * @brief exportGPIO method calls export based on the member variables, _name and _path.
	 * @return 0 for success, -1 for failure
	 */
	int exportGPIO();

	/**
	 * @brief unexportGPIO method calls unexport based on the member variables, _name and _path.
	 * @return 0 for success, -1 for failure
	 */
	int unexportGPIO();
};

} // namespace IO
} // namespace Core

#endif // IOBASE_H_
