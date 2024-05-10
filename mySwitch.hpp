#ifndef MYSWITCH_H
#define MYSWITCH_H

#include "myActuator.hpp"

void initializeSwitches();
void checkPowerSwitch(Actuator actuator);
void checkManualSwitch(Actuator actuator);
void checkZeroSwitch(Actuator actuator);

#endif