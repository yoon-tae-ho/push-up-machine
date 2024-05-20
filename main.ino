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

#define MAX_INT_VALUE 2147483647

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

//* Constants for Actuator
double moveStartTime = MAX_INT_VALUE;
double moveRefTime = 5000; // ms

int pivot = 5;
int count = 0;
int moveCount = 0;

// TODO: 민재
double refChangeUp = 0.16 * (MAX_ACTUATOR_SPEED * moveRefTime / 1000) / 300;
double refChangeDown = 0.14 * (MAX_ACTUATOR_SPEED * moveRefTime / 1000) / 300;
double loadRatio = 1.0;
double repFactor = 0.7; // ref_load에 대한 비율, 0 ~ 1


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

      double maxRefLoad = localExtreme[0] * repFactor;
      double minRefLoad = localExtreme[1] / repFactor;
      user.setRefLoad(maxRefLoad, minRefLoad);

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
    // TODO: 민재
    // maxRefLoad *= loadRatio;
    // minRefLoad *= loadRatio;

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
    count = user.getCount();
    if ((count % pivot == 0) && (count != 0) && (count != moveCount)) {
      actuator.setBackward();
      actuator.actuate(MAX_ACTUATOR_PWM);
      moveStartTime = checkTime; //ms
      moveCount = count; // 여러 번 실행되지 않게 하기 위해.
    }
    if ((checkTime - moveStartTime >= moveRefTime) && actuator.isWorking) {
      actuator.actuate(0);
      moveStartTime = MAX_INT_VALUE;

      // TODO: 민재
      // moveDown ? loadRatio += refChangeUp : loadRatio -= refChangeDown;
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