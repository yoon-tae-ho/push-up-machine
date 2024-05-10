// mySwitch.cpp
#include "mySwitch.hpp"
#include "myGpio.hpp"
#include "myActuator.hpp"
#include <Arduino.h>

//* Switch 값이 HIGH(1)일 경우 off
//* Switch 값이 LOW(0)일 경우 on

bool manualUpBefore;
bool manualDownBefore;
bool zeroBefore;

bool manualUp;
bool manualDown;
bool zero;

void initializeSwitches(bool manualUp, bool manualDown, bool zero) {
  manualUpBefore = manualUp;
  manualDownBefore = manualDown;
  zeroBefore = zero;
}

void checkPowerSwitch(Actuator actuator) {
  actuator.setAvailable(digitalRead(POWER) == LOW ? true : false);
}

void checkManualSwitch(Actuator actuator) {
  if ((manualUp == manualUpBefore) && (manualDown == manualDownBefore)) return;

  manualUp = digitalRead(MANUAL_ACTUATOR_UP);
  manualDown = digitalRead(MANUAL_ACTUATOR_DOWN);

  if (manualUp == LOW && manualDown == HIGH) {
    // actuate upward manualy
    actuator.setManualing(true);
    actuator.setForward();
    actuator.actuate(MAX_ACTUATOR_PWM, true);
  } else if (manualUp == HIGH && manualDown == LOW) {
    // actuate downward manualy
    actuator.setManualing(true);
    actuator.setBackward();
    actuator.actuate(MAX_ACTUATOR_PWM, true);
  } else {
    actuator.setManualing(false);
  }

  manualUpBefore = manualUp;
  manualDownBefore = manualDown;
}

void checkZeroSwitch(Actuator actuator) {
  zero = digitalRead(ZERO_ADJUSTMENT);
  if (zeroBefore == HIGH && zero == LOW) {
    //TODO: start zero adjustment
  }
  zeroBefore = zero;
}
