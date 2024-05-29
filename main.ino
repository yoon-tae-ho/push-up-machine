#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <EEPROM.h>

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "HX711.h"
#include "myActuator.hpp"
#include "myGpio.hpp"
#include "myLoadcell.hpp"
#include "mySwitch.hpp"
#include "myUtils.hpp"
#include "myUser.hpp"

#define MAX_INT_VALUE 2147483647

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "D7766EBB-08AE-47F6-81CD-E8E1E7A570F8"
#define CHARACTERISTIC_UUID2 "DF6D201B-CDA2-4B19-ADD4-BC82323D117B"

Loadcell loadcell;
Actuator actuator;
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
double currentPos;
double refPosition = 0;

//* for BLE
BLECharacteristic *pCharacteristicRead;
BLECharacteristic *pCharacteristicWrite;
bool deviceConnected = false;
String readVariable;  // read from esp
String writeVariable; // write to esp
double prevTime;

String firstRead;
String secondRead;
String thirdRead;
String forthRead;
String fifthRead;
String sixthRead;
String seventhRead;
String eigthRead;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) { deviceConnected = true; }

  void onDisconnect(BLEServer *pServer) { deviceConnected = false; }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      writeVariable = String(value.c_str());
    }
  }
};

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
  double pos = loadValue(EEPROM_POSITION_INDEX);
  actuator.setInitialPosition(pos);

  //* Switch
  initializeSwitches();

  //* BLE
  BLEDevice::init("ESP32_YoonTaeHo BABO");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristicRead = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  pCharacteristicRead->addDescriptor(new BLE2902());
  pCharacteristicRead->setValue(String(readVariable).c_str());

  pCharacteristicWrite = pService->createCharacteristic(
      CHARACTERISTIC_UUID2, BLECharacteristic::PROPERTY_WRITE);

  pCharacteristicWrite->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();

  //* User
  user = User();

  // load reference load from flash memory
  user.setRefLoad(loadValue(EEPROM_ZERO_MAX_INDEX), loadValue(EEPROM_ZERO_MIN_INDEX), pos);
}

void loop() {
  checkTime = millis();
  duringTime = (double)(checkTime - startTime) / 1000.0; // elapsed time
  timeStep = (double)(checkTime - checkTimeBefore) / 1000.0;
  currentPos = actuator.getCurrentPosition();

  //* Switch
  checkPowerSwitch(actuator);
  checkManualSwitch(actuator);
  if (checkZeroSwitch()) {
    zeroAdjustmenting = (digitalRead(ZERO_ADJUSTMENT) == LOW);
    // stop zero adjustment
    if (!zeroAdjustmenting) {
      std::vector<double> localExtreme = getLocalExtremeValue(dataForZeroAdj, windowSize);

      double minRefLoad = localExtreme[1] / 0.95; //rep factor
      double maxRef1 = localExtreme[0];
      double maxRef2 = localExtreme[1] * 1.5; //ref ratio
      double maxRefLoad = (maxRef1 * 0.3 + maxRef2 * (1 - 0.3)) * 0.95; //zero ratio
      user.setRefLoad(maxRefLoad, minRefLoad, currentPos);
      //최저 높이 reference
      //save at flash memory (추후에 높이 보정해야함, calculateRef())
      saveValue(EEPROM_ZERO_MAX_INDEX, user.getMaxRefLoad());
      saveValue(EEPROM_ZERO_MIN_INDEX, user.getMinRefLoad());
    }
  }
    // reference load를 높이에 맞게 바꿔야 함
    user.calculateRef(currentPos);

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
        if (user.autoMode) user.levelCount += 1;
        user.totalCount += 1;
        user.wentDown = false;
      }
    }

    if (user.autoMode) {
      // 몇 번 이후 작동
      if ((user.levelCount == 5) && (user.levelCount != 0)) {
        actuator.setBackward();
        actuator.actuate(MAX_ACTUATOR_PWM);
        refPosition = currentPos;
        user.levelCount = 0;
      }

      if ((fabs(refPosition - currentPos) >= 100.0) && actuator.isWorking) {
        actuator.actuate(0);
      }
    }
  }

  // calculate position of actuator
  actuator.calculatePosition(timeStep);

  //* BLE
  // BLE example
  String result = "";

  // First read
  result += (digitalRead(POWER) == LOW) ? "0" : "1";
  result += ",";
  // Second read
  result += !actuator.isWorking ? "0" : (actuator.direction == HIGH ? "2" : "1");
  result += ",";
  // Third read
  result += (digitalRead(ZERO_ADJUSTMENT) == HIGH) ? "0" : "1";
  result += ",";
  // Fourth read
  result += String(user.totalCount);
  result += ",";
  // Fifth read
  result += String(actuator.getCurrentPosition());
  result += ",";
  // Sixth read
  result += (loadcell.checkBalance()) ? "0" : (loadcell.getLeftLoad() > loadcell.getRightLoad()) ? "1" : "2";
  result += ",";
  // Seventh read
  result += "1";  // Assuming pose is always good
  result += ",";
  // Eighth read
  result += String(user.getState());

  if (checkTime - prevTime >= 100) {
    if (deviceConnected) {
      pCharacteristicRead->setValue(result.c_str());
      pCharacteristicRead->notify();
      if (writeVariable.length() > 0) {
        if (writeVariable.indexOf("U") >= 0) {
          // manual up
          actuator.setManualing(true);
          actuator.setForward();
          actuator.actuate(MAX_ACTUATOR_PWM, true);
        } else if (writeVariable.indexOf("S") >= 0) {
          // manual stop
          actuator.actuate(0, true);
          actuator.setManualing(false); 
        } else if (writeVariable.indexOf("D") >= 0) {
          // manual down
          actuator.setManualing(true);
          actuator.setBackward();
          actuator.actuate(MAX_ACTUATOR_PWM, true);
        }
        // Clear writeVariable after processing to avoid repeated actions
        writeVariable = "";
      }
    }
  prevTime = checkTime;
  }
// 

  checkTimeBefore = checkTime;
  loadBefore = loadSum;
}
