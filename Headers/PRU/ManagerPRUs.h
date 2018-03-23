/*
 * Licensed under the 'LICENSE'.
 * See LICENSE file in the project root for full license information.
 */
#ifndef MANAGERPRUS_H
#define MANAGERPRUS_H

#include <stdint.h>

namespace LibBBB  {

/**
 * @brief	The ManagerPRUs class manages BOTH PRUs for the application. This ensures
 *			that NO other part of the application can control the PRUs. In a basic
 *			sense, this class forces access to the pointer to PRU memory to come
 *			through it as a way of protecting a VERY important pointer. This
 *			forces the pointer's value to a constant that nothing but this class
 *			can modify.
 */
class ManagerPRUs
{
private:

	/**
	 * @brief ManagerPRUs contructs the singleton instance of the class
	 */
	ManagerPRUs();

private:

	// Member variables
	bool _pruRunning;
	uint32_t* _pruSharedMemoryPointer;

public:

	/**
	 * @brief	instance method is the Singleton method of creating and accessing this
	 *			class. This ensures there can only be ONE instance of this class. If there
	 *			are two instances of this class created, it could mess with the virtual
	 *			memory mapping!!
	 * @note	https://en.wikipedia.org/wiki/Singleton_pattern
	 * @return reference to this class.
	 */
	static ManagerPRUs& instance();

	/**
	 * @brief ~ManagerPRUs unbinds the PRUs, shutting them down
	 */
	~ManagerPRUs();

	/**
	 * @brief restartPRUs restart the PRUs
	 * @return 0 if successful
	 */
	int restartPRUs();

	/**
	 * @brief setPRUState turns the BOTH PRUs on or off
	 * @param on
	 * @return false for no errors
	 */
	int setPRUState( bool on );

	/**
	 * @brief	sharedMemoryPointer gets the pointer to the shared memory
	 *			pointer. This class has NO error checking method to ensure
	 *			the pointer is used correctly. Most times the pointer is used
	 *			as an array, so there is no error checking to make sure the
	 *			app does not make a segmentation fault.
	 * @note	Both getter methods do NOT allow the pointer to be changed to
	 *			ensure it does not get moved to a position that will cause
	 *			a segmentation fault.
	 * @return pointer to the shared PRU memory
	 */
	uint32_t* const sharedMemoryPointer();
	const uint32_t* const sharedMemoryPointer() const;

private:

	/**
	 * @brief bindOrUnbindPRU binds or unbinds the PRUs. Essentially, turnin them on and off.
	 * @param bind true to bind them (turn the on), false to unbind them (turn them off).
	 * @return TODO:
	 */
	int bindOrUnbindPRU( bool bind );

	/**
	 * @brief	mapVirtualAddressSpace maps the virtual memory space to set up the communication
	 *			to the PRUs, then sets the _pruSharedMemoryPointer to the proper position.
	 * @return
	 */
	int mapVirtualAddressSpace();
};

} // namespace LibBBB
#endif // MANAGERPRUS_h
