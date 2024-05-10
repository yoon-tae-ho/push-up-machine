// myLoadcell.c
#include "HX711.h"
#include "myLoadcell.hpp"
#include "myGpio.hpp"
#include <Arduino.h>

Loadcell::Loadcell() {
  scaleRight = HX711();
  scaleRight.begin(DOUT_RIGHT, CLK_RIGHT); 
  scaleRight.set_scale(calibration_factor); 
  scaleRight.tare();   //영점잡기. 현재 측정값을 0으로 둔다.
  scaleLeft = HX711();
  scaleLeft.begin(DOUT_LEFT, CLK_LEFT); 
  scaleLeft.set_scale(calibration_factor); 
  scaleLeft.tare();   //영점잡기. 현재 측정값을 0으로 둔다.
}

double Loadcell::getRightLoad() {
  return (double)scaleRight.get_units();
}

double Loadcell::getLeftLoad() {
  return (double)scaleLeft.get_units();
}