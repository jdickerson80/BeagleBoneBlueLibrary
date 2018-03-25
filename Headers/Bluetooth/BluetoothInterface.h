#ifndef BLUETOOTHINTERFACE_H
#define BLUETOOTHINTERFACE_H

#include "BluetoothManager.h"

namespace LibBBB {
namespace Bluetooth {

class Manager::Interface
{
public:

    Interface(){}

    ~Interface(){}
    virtual int stateChange( Manager::State::Enum newState ) = 0;
};


} // namespace Bluetooth
} // namespace LibBBB
#endif // BLUETOOTHINTERFACE_H
