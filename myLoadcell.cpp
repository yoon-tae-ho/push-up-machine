// myLoadcell.c
#include "myLoadcell.hpp"
#include "HX711.h"
#include "myGpio.hpp"
#include <Arduino.h>

Loadcell::Loadcell() {
  scaleRight = HX711(DOUT_RIGHT, CLK_RIGHT);
  scaleRight.set_scale(calibration_factor);
  scaleRight.tare(); //영점잡기. 현재 측정값을 0으로 둔다.
  scaleLeft = HX711(DOUT_LEFT, CLK_LEFT);
  scaleLeft.set_scale(calibration_factor);
  scaleLeft.tare(); //영점잡기. 현재 측정값을 0으로 둔다.

  isBalanced = true;
  balanceFactor = 0.85;
}

float Loadcell::getRightLoad() { return (float)scaleRight.get_units(); }

float Loadcell::getLeftLoad() { return (float)scaleLeft.get_units(); }

bool Loadcell::checkBalance() {
  float loadRight = getRightLoad();
  float loadLeft = getLeftLoad();

  return (loadLeft > loadRight * balanceFactor) ||
         (loadRight > loadLeft * balanceFactor);
}

bool Loadcell::checkBalanceDirection() {
  // unbalance 일 때만!!
  return getLeftLoad() < getRightLoad();
}