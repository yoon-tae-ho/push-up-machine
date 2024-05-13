#include "HX711.h"
#include "myActuator.hpp"
#include "myLoadcell.hpp"
#include "mySwitch.hpp"
#include "myGpio.hpp"

Loadcell loadcell;
Actuator actuator;

//* Constants
int startTime = 0;
int checkTime = 0;
int checkTimeBefore = 0;
double duringTime = 0;
double timeStep = 0;

//* Constants for Loadcell
double load_right = 0.0;
double load_left = 0.0;

//* Constants for Actuator
int count = 0;
double referenceValue = 10.0;
bool isOver = false;

void setup() {
  Serial.begin(115200);  // 값 모니터링 위해서...

  startTime = millis();
  checkTimeBefore = startTime;

  //* Loadcell
  loadcell = Loadcell();

  //* Actuator
  actuator = Actuator();

  //* Switch
  initializeSwitches();
}

void loop() {
  checkTime = millis();
  duringTime = (double)(checkTime - startTime) / 1000.0;
  timeStep = (double)(checkTime - checkTimeBefore) / 1000.0;

  //* Loadcell
  load_right = loadcell.getRightLoad();
  load_left = loadcell.getLeftLoad();
  Serial.print("Right: ");
  Serial.println(load_right);
  Serial.print("Left: ");
  Serial.println(load_left);
  Serial.println("======================================");

  //* Actuator
  // if (load_left > referenceValue) {
  //   isOver = true;
  // } else {
  //   if (isOver) {
  //       ++count;
  //   }
  //   isOver = false;
  // }

  // if (count >= 3) {
  //   actuator.setForward();
  //   actuator.actuate(MAX_ACTUATOR_PWM);
  //   delay(5000);
  //   actuator.actuate(0);
  //   count = 0;
  // }

  Serial.print("isAvailable: ");
  Serial.println(actuator.isAvailable);
  Serial.print("isManualing: ");
  Serial.println(actuator.isManualing);
  Serial.print("isWorking: ");
  Serial.println(actuator.isWorking);

  // calculate position of actuator
  actuator.calculatePosition(timeStep);

  //* Switch
  checkPowerSwitch(actuator);
  checkManualSwitch(actuator);
  checkZeroSwitch(actuator);

  checkTimeBefore = checkTime;
}
