#ifndef BLUETOOTHINTERFACE_H
#define BLUETOOTHINTERFACE_H

#include "BluetoothManager.h"

namespace LibBBB {
namespace Bluetooth {

/**
 * @brief	The Manager::Interface class is the abstract class that represents the
 *			interface to the Bluetooth manager. This class should be inherited by
 *			any class that wishes to get events from the Bluetooth manager.
 */
class Manager::Interface
{
public:

	/**
	 * @brief Interface do nothing constructor
	 */
    Interface(){}

	/**
	 * @brief ~Interface do nothing destructor
	 */
	~ Interface(){}

	/**
	 * @brief	stateChange method that the concrete class uses to get the
	 *			events from the Bluetooth manager
	 * @param newState that the Bluetooth manager is entering
	 * @return nothing important
	 */
    virtual int stateChange( Manager::State::Enum newState ) = 0;
};


} // namespace Bluetooth
} // namespace LibBBB
#endif // BLUETOOTHINTERFACE_H
