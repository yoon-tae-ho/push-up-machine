// mySwitch.cpp
#include "mySwitch.hpp"
#include "myActuator.hpp"
#include "myGpio.hpp"
#include <Arduino.h>

//* Switch 값이 HIGH(1)일 경우 off
//* Switch 값이 LOW(0)일 경우 on

bool manualUpBefore;
bool manualDownBefore;
bool zeroBefore;

bool manualUp;
bool manualDown;
bool zero;

void initializeSwitches() {
  pinMode(POWER, INPUT_PULLUP);
  pinMode(MANUAL_ACTUATOR_UP, INPUT_PULLUP);
  pinMode(MANUAL_ACTUATOR_DOWN, INPUT_PULLUP);
  pinMode(ZERO_ADJUSTMENT, INPUT_PULLUP);

  manualUpBefore = digitalRead(MANUAL_ACTUATOR_UP);
  manualDownBefore = digitalRead(MANUAL_ACTUATOR_DOWN);
  zeroBefore = digitalRead(ZERO_ADJUSTMENT);
}

void checkPowerSwitch(Actuator &actuator) {
  // actuator.setAvailable(digitalRead(POWER) == LOW);
  actuator.setAvailable(true);
}

void checkManualSwitch(Actuator &actuator) {
  manualUp = digitalRead(MANUAL_ACTUATOR_UP);
  manualDown = digitalRead(MANUAL_ACTUATOR_DOWN);

  if ((manualUp == manualUpBefore) && (manualDown == manualDownBefore))
    return;

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
    actuator.actuate(0, true);
    actuator.setManualing(false);
  }

  manualUpBefore = manualUp;
  manualDownBefore = manualDown;
}

bool checkZeroSwitch() {
  zero = digitalRead(ZERO_ADJUSTMENT);
  bool result;
  if (zeroBefore == HIGH && zero == LOW) {
    result = true;
  } else {
    result = false;
  }
  zeroBefore = zero;
  return result;
}
