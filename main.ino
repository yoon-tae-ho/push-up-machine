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
float duringTime = 0;
float timeStep = 0;

//* Constants for Loadcell
std::vector<float> dataForZeroAdj;
const int windowSize = 5;
bool zeroInterrupt = false;
bool zeroAdjustmenting = false;

//* Constants for Loadcell
float loadRight = 0.0;
float loadLeft = 0.0;
float loadSum = 0.0;
float loadBefore = 0.0;

//* Constants for Actuator
float currentPos;
float refPosition = 0;
bool badRange = false;

//* for BLE
BLECharacteristic *pCharacteristicRead;
BLECharacteristic *pCharacteristicWrite;
bool deviceConnected = false;
String readVariable;  // read from esp
String writeVariable; // write to esp
float prevTime;


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
  float pos = loadValue(EEPROM_POSITION_INDEX);
  actuator.setInitialPosition(pos);

  //* Switch
  initializeSwitches();

  //* BLE
  BLEDevice::init("CANADIAN YOON_TAEHO");

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
  duringTime = (float)(checkTime - startTime) / 1000.0; // elapsed time
  timeStep = (float)(checkTime - checkTimeBefore) / 1000.0;
  currentPos = actuator.getCurrentPosition();

  //* Switch
  checkPowerSwitch(actuator);
  checkManualSwitch(actuator);
  if (zeroInterrupt) {
    zeroAdjustmenting = !zeroAdjustmenting;
    if (!zeroAdjustmenting) {
      std::vector<float> localExtreme = getLocalExtremeValue(dataForZeroAdj, windowSize);

      float minRefLoad = localExtreme[1] / 0.95; //rep factor
      float maxRef1 = localExtreme[0];
      float maxRef2 = localExtreme[1] * 1.5; //ref ratio
      float maxRefLoad = (maxRef1 * 0.3 + maxRef2 * (1 - 0.3)) * 0.95; //zero ratio
      user.setRefLoad(maxRefLoad, minRefLoad, currentPos);
      //최저 높이 reference
      //save at flash memory (추후에 높이 보정해야함, calculateRef())
      saveValue(EEPROM_ZERO_MAX_INDEX, user.getMaxRefLoad());
      saveValue(EEPROM_ZERO_MIN_INDEX, user.getMinRefLoad());

      // clear vector dataForZeroAdj
      dataForZeroAdj.clear();
    }
    zeroInterrupt = !zeroInterrupt;
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
        user.wentDown = true;
        badRange = false;
      }

      // 덜 올라간 후 내려옴 or 덜 내려간 후 올라감
      if ((user.getPprevState() == 2 && user.getPrevState() == 1 && user.getState() == 2) || 
          (user.getPprevState() == 1 && user.getPrevState() == 2 && user.getState() == 1)) {
        user.wentDown = false;
        badRange = true;
      }

      // count 요건
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
        actuator.actuate(true);
        refPosition = currentPos;
        user.levelCount = 0;
      }
      if ((fabs(refPosition - currentPos) >= 50.0) && actuator.isWorking) {
        actuator.actuate(false);
      }
    }
  }

  // calculate position of actuator
  actuator.calculatePosition(timeStep);

  //* BLE
  String result = "";
  // 1: POWER, MANUAL
  int power = actuator.isAvailable ? 3 : 0;
  int manual = !actuator.isWorking ? 0 : (actuator.direction == HIGH ? 2 : 1);
  result += String(power + manual);
  result += ",";
  // 2: ZERO, BALANCE
  int zero = zeroAdjustmenting ? 3 : 0;
  int balance = (loadcell.checkBalance()) ? 0 : (loadcell.getLeftLoad() > loadcell.getRightLoad()) ? 1 : 2;
  result += String(zero + balance);
  result += ",";
  // 3: BAD POSE, BAD RANGE
  int pose = 0; // 안 좋은게 2
  int range = badRange ? 1 : 0;
  result += String(pose + range);
  result += ",";
  // 4: TOTAL COUNT
  result += String(user.totalCount);
  result += ",";
  // 5: POSITION
  result += String(actuator.getCurrentPosition());
  result += ",";
  // 6: STATE(PROGRESS)
  result += String(user.getState());
  result += ",";
  // 7: LEVEL COUNT
  result += String(user.levelCount);
  Serial.println(result);

  if (checkTime - prevTime >= 100) {


    if (deviceConnected) {
      pCharacteristicRead->setValue(result.c_str());
      pCharacteristicRead->notify();
      if (writeVariable.length() > 0) {
        if (writeVariable.indexOf("U") >= 0) {
          // manual up
          actuator.setManualing(true);
          actuator.setForward();
          actuator.actuate(true, true);
        } else if (writeVariable.indexOf("S") >= 0) {
          // manual stop
          actuator.actuate(false, true);
          actuator.setManualing(false); 
        } else if (writeVariable.indexOf("D") >= 0) {
          // manual down
          actuator.setManualing(true);
          actuator.setBackward();
          actuator.actuate(true, true);
        } else if (writeVariable.indexOf("Z") >= 0) {
          // zero adjustment
          zeroInterrupt = true;
        } else if (writeVariable.indexOf("A") >= 0) {
          user.autoMode = !user.autoMode;
        }
        // Clear writeVariable after processing to avoid repeated actions
        writeVariable = "";
      }
    }
    prevTime = checkTime;
  }

  actuator.checkHeightLimit();
  checkTimeBefore = checkTime;
  loadBefore = loadSum;
}
