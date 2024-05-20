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
  count = 0;

  wentDown = false;
}

void User::setRefLoad(const double& max, const double& min) {
  maxRefLoad = max;
  minRefLoad = min;
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

void User::setCount(const int& newCount) {
  count = (newCount < 0) ? 0 : newCount;
}

int User::getCount() {
  return count;
}
