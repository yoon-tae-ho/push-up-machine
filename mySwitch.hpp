#ifndef MYSWITCH_H
#define MYSWITCH_H

#include "myActuator.hpp"

void initializeSwitches(bool manualUp, bool manualDown, bool zero);
void checkPowerSwitch(Actuator actuator);
void checkManualSwitch(Actuator actuator);
void checkZeroSwitch(Actuator actuator);

#endif