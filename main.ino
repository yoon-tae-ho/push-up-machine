#include "HX711.h"
#include "myActuator.hpp"
#include "myGpio.hpp"
#include "myLoadcell.hpp"
#include "mySwitch.hpp"
#include "myUtils.hpp"
#include <EEPROM.h>

#define EEPROM_POSITION_INDEX 0
#define EEPROM_ZERO_MAX_INDEX 1
#define EEPROM_ZERO_MIN_INDEX 2

Loadcell loadcell;
Actuator actuator;

//* Constants
int startTime = 0;
int checkTime = 0;
int checkTimeBefore = 0;
double duringTime = 0;
double timeStep = 0;
bool zeroAdjustment = false;
bool zeroSwitch = false;

//* Constants for Loadcell
double loadRight = 0.0;
double loadLeft = 0.0;
double loadAvg = 0.0;
double loadBefore = 0.0;
double minimumValue = 3.0;

bool wentDown = false;
bool wentUp = false;
bool zeroDone;
bool bottom = false;
bool top = false;
bool goingUp = false;
bool goingDown = false;

//* Constants for Actuator
double moveRepsTime = 1000000;
double movingTime = 3000; // ms
bool isMove = false;
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
double userMax;
double userMin; //하중 계속 측정


void afterOneRep() {
  moveCount++;
  count++;
  Serial.println(count);
  userMax = (maxRefLoad + minRefLoad) / 2;
  userMin = (maxRefLoad + minRefLoad) / 2;
  wentUp = false;
  wentDown = false;
  top = false;
  bottom = false;
  goingDown = false;
  goingUp = false;
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  startTime = millis();
  checkTimeBefore = startTime;

  //* Loadcell
  loadcell = Loadcell();

  //* Actuator
  actuator = Actuator();
  actuator.setCurrentPosition(loadValue(EEPROM_POSITION_INDEX));

  //* Switch
  initializeSwitches();
}

void loop() {
  checkTime = millis();
  duringTime = (double)(checkTime - startTime) / 1000.0; // elapsed time
  timeStep = (double)(checkTime - checkTimeBefore) / 1000.0;

  //* Loadcell
  loadRight = loadcell.getRightLoad();
  loadLeft = loadcell.getLeftLoad();
  loadAvg = (loadLeft + loadRight) / 2;

  //* Actuator
  zeroDone = (maxRefLoad > minRefLoad) ? true : false;

  // zero adjustment on/off
  if (checkZeroSwitch()) {
    if (zeroAdjustment) {
      zeroAdjustment = false;
      if (!zeroDone)
        Serial.println("Need to re-do adjustment");
      maxLoad = 0.0;
      minLoad = 300.0;
      Serial.println("Adjustment Done");
    } else {
      zeroAdjustment = true;
      Serial.println("Adjustment Started");
    }
  }

  // Zero Adjustment
  if (zeroAdjustment && loadcell.checkBalance()) {
    if (loadAvg > maxLoad) {
      maxLoad = loadAvg;
    }
    if (loadAvg < minLoad && loadAvg > minimumValue) {
      minLoad = loadAvg;
    }
    maxRefLoad = maxLoad * repFactor;
    minRefLoad = minLoad / repFactor;
    userMax = (maxRefLoad + minRefLoad) / 2;
    userMin = (maxRefLoad + minRefLoad) / 2;
  }

  userMax = (loadAvg > userMax) ? loadAvg : userMax;
  userMin = (loadAvg < userMin) ? loadAvg : userMin;

  maxLoad *= loadRatio;
  minLoad *= loadRatio;
  
  // counting
  if (!isMove && zeroDone && !zeroAdjustment && loadcell.checkBalance()) {   
    if (loadAvg > maxRefLoad){
      wentDown = true;
      bottom = goingUp ? true : false;
    }
    if (loadAvg < maxRefLoad && wentDown)
      goingUp = true;
    if (loadAvg > minRefLoad && !wentDown)
      goingDown = true;
    if (loadAvg < minRefLoad){
      wentUp = wentDown ? true : false;
      top = (goingDown && !wentDown) ? true : false;
    }
    if (wentDown && wentUp)//한번하고 나면
      afterOneRep();
  }

  // 몇 번 이후 작동
  bool moveReps = (moveCount == everyCount) ? true : false;
  if (moveReps) {
    moveDown ? actuator.setForward() : actuator.setBackward();
    actuator.actuate(MAX_ACTUATOR_PWM);
    moveRepsTime = checkTime; //ms
    moveReps = false;
    isMove = true;
    moveCount = 0;
    Serial.print("Moving from ");
    Serial.println(actuator.getCurrentPosition());
  } else if (checkTime - moveRepsTime >= movingTime && isMove) {
    isMove = false;
    actuator.actuate(0);
    Serial.print("Height adjusted to ");
    Serial.println(actuator.getCurrentPosition());
    moveDown ? loadRatio += refChangeUp : loadRatio -= refChangeDown;
  }

  // calculate position of actuator
  actuator.calculatePosition(timeStep);

  //* Switch
  checkPowerSwitch(actuator);
  checkManualSwitch(actuator);

  checkTimeBefore = checkTime;
  loadBefore = loadAvg;
}