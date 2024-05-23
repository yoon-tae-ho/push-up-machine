// myUser.cpp
#include "myUser.hpp"
#include "myGpio.hpp"
#include <Arduino.h>

// private

// public

User::User() {
  maxRefLoad = 0;
  minRefLoad = 0;

  state = 0;
  prevState = 0;
  totalCount = 0;
  levelCount = 0;

  double refDiff = 0.16 / 300; // max가 min의 몇 배인지
  
  wentDown = false;
}

void User::setRefLoad(const double& max, const double& min, const double& currentPos) {  
  maxRefLoad = max / (1 + refDiff * currentPos);
  minRefLoad = min / (1 + refDiff * currentPos);
}

void User::calculateRef(const double& currentPos) {
  maxRefLoad *= (1 + refDiff * currentPos);
  minRefLoad *= (1 + refDiff * currentPos);
}

double User::getMaxRefLoad() {
  return maxRefLoad;
}

double User::getMinRefLoad() {
  return minRefLoad;
}

double User::getMidRefLoad() {
  return (maxRefLoad + minRefLoad) / 2.0;
}

void User::setState(const int& newState) {
  // (0 ~ 3 : up ~ down)
  if (state == newState || newState < 0 || newState > 3) return;
  prevState = state;
  state = newState;
}

int User::getState() {
  return state;
}

int User::getPrevState() {
  return prevState;
}