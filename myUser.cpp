// myUser.cpp
#include "myUser.hpp"
#include "myGpio.hpp"
#include <Arduino.h>

// private

// public

User::User() {
  maxRefLoad = 0;
  minRefLoad = 0;
  realMax = 0; //최저 높이의 reference
  realMin = 0;

  state = 0;
  prevState = 0;
  pprevState = 0;
  totalCount = 0;
  levelCount = 0;

  autoMode = true;

  refDiff = 0.16 / 300; // (액추에이터 높이)높은게 낮은거의 80%일 때 0.2/300 [per mm]

  wentDown = false;
}

void User::setRefLoad(const float &max, const float &min,
                      const float &currentPos) {
  // 진짜 reference(최저 높이라고 가정, 최고 하중, 값이 증가해야 함)
  realMax = max / (1 - refDiff * currentPos);
  realMin = min / (1 - refDiff * currentPos);
}

void User::calculateRef(const float &currentPos) {
  // 현재 높이에 맞게 reference 세팅(하중 낮아짐)
  maxRefLoad = realMax * (1 - refDiff * currentPos);
  minRefLoad = realMin * (1 - refDiff * currentPos);
}

float User::getMaxRefLoad() { return maxRefLoad; }

float User::getMinRefLoad() { return minRefLoad; }

float User::getMidRefLoad() { return (maxRefLoad + minRefLoad) / 2.0; }

void User::setState(const int &newState) {
  // (0 ~ 3 : up ~ down)
  if (state == newState || newState < 0 || newState > 3)
    return;
  pprevState = prevState;
  prevState = state;
  state = newState;
}

int User::getState() { return state; }

int User::getPrevState() { return prevState; }

int User::getPprevState() { return pprevState; }