#include <iostream>
#include <vector>
#include <algorithm>
#include <EEPROM.h>

#include "HX711.h"
#include "myActuator.hpp"
#include "myGpio.hpp"
#include "myLoadcell.hpp"
#include "mySwitch.hpp"
#include "myUtils.hpp"
#include "myBLE.hpp"
#include "myUser.hpp"

#define EEPROM_POSITION_INDEX 0
#define EEPROM_ZERO_MAX_INDEX 1
#define EEPROM_ZERO_MIN_INDEX 2

Loadcell loadcell;
Actuator actuator;
BLEHandler bleHandler;
User user;

//* Constants
int startTime = 0;
int checkTime = 0;
int checkTimeBefore = 0;
double duringTime = 0;
double timeStep = 0;

//* Constants for Loadcell
std::vector<double> dataForZeroAdj;
const int windowSize = 5;
bool zeroAdjustmenting = false;

//* Constants for Loadcell
double loadRight = 0.0;
double loadLeft = 0.0;
double loadSum = 0.0;
double loadBefore = 0.0;
double minimumValue = 3.0;

//* Constants for Actuator
double moveRepsTime = 1000000;
double movingTime = 3000; // ms
bool moveUp = false;
bool moveDown = true;

int count = 0;
int moveCount = 0;
int everyCount = 5;
double maxLoad = 0.0;
double minLoad = 300.0;
double maxRefLoad = 0.0; //실제 사용할 최대값
double minRefLoad = 0.0; //실제 사용할 최소값
double refChangeUp = 0.16 * (MAX_ACTUATOR_SPEED * movingTime / 1000) / 300;
double refChangeDown = 0.14 * (MAX_ACTUATOR_SPEED * movingTime / 1000) / 300;
double loadRatio = 1.0;

double repFactor = 0.7; // ref_load에 대한 비율, 0 ~ 1

//* Constants for User


void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  startTime = millis();
  checkTimeBefore = startTime;

  //* Loadcell
  loadcell = Loadcell();

  //* Actuator
  actuator = Actuator();

  // load initial position from flash memory
  actuator.setInitialPosition(loadValue(EEPROM_POSITION_INDEX));

  //* Switch
  initializeSwitches();

  //* BLE
  bleHandler = BLEHandler("ESP32_BLE_Server", SERVICE_UUID, CHARACTERISTIC_UUID);
  bleHandler.init();

  //* User
  user = User();

  // load reference load from flash memory
  user.setRefLoad(loadValue(EEPROM_ZERO_MAX_INDEX), loadValue(EEPROM_ZERO_MIN_INDEX));
}

void loop() {
  checkTime = millis();
  duringTime = (double)(checkTime - startTime) / 1000.0; // elapsed time
  timeStep = (double)(checkTime - checkTimeBefore) / 1000.0;

  //* Switch
  checkPowerSwitch(actuator);
  checkManualSwitch(actuator);
  if (checkZeroSwitch()) {
    zeroAdjustmenting = (dititalRead(ZERO_ADJUSTMENT) == LOW);
    // stop zero adjustment
    if (!zeroAdjustmenting) {
      std::vector<double> localExtreme = getLocalExtremeValue(dataForZeroAdj, windowSize);

      maxRefLoad = localExtreme[0] * repFactor;
      minRefLoad = localExtreme[1] / repFactor;

      // save at flash memory
      saveValue(EEPROM_ZERO_MAX_INDEX, maxRefLoad);
      saveValue(EEPROM_ZERO_MIN_INDEX, minRefLoad);
    }
  }

  //* Loadcell
  loadRight = loadcell.getRightLoad();
  loadLeft = loadcell.getLeftLoad();
  loadSum = loadLeft + loadRight;

  //* Actuator
  if (zeroAdjustmenting) {
    //* Zero Adjustment
    dataForZeroAdj.push_back(loadSum);
  } else {
    //* Main Function
    maxRefLoad *= loadRatio;
    minRefLoad *= loadRatio;

    // counting
    if (loadcell.checkBalance()) {
      // User.state -> (0 ~ 3 : up ~ down)
      if (loadSum < user.getMinRefLoad()) {
        user.setState(0);
      } else if (loadSum < user.getMidRefLoad()) {
        user.setState(1);
      } else if (loadSum < user.getMaxRefLoad()) {
        user.setState(2);
      } else {
        user.setState(3);
        if (user.getPrevState() == 2) user.wentDown = true;
      }

      if (user.wentDown && user.getState() == 0) {
        user.setCount(user.getCount() + 1);
        user.wentDown = false;
      }
    }

    // 몇 번 이후 작동
    bool moveReps = (moveCount == everyCount) ? true : false;
    if (moveReps) {
      moveDown ? actuator.setForward() : actuator.setBackward();
      actuator.actuate(MAX_ACTUATOR_PWM);
      moveRepsTime = checkTime; //ms
      moveReps = false;
      moveCount = 0;
      Serial.print("Moving from ");
      Serial.println(actuator.getCurrentPosition());
    } else if (checkTime - moveRepsTime >= movingTime && actuator.isWorking) {
      actuator.actuate(0);
      Serial.print("Height adjusted to ");
      Serial.println(actuator.getCurrentPosition());
      moveDown ? loadRatio += refChangeUp : loadRatio -= refChangeDown;
    }
  }

  // calculate position of actuator
  actuator.calculatePosition(timeStep);

  //* BLE
  // BLE example
  if (bleHandler.getIsConnected()) {
    bleHandler.notify("minjae,ba,bo");
  }

  checkTimeBefore = checkTime;
  loadBefore = loadSum;
}