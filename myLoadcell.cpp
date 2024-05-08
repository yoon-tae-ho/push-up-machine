// myLoadcell.c
#include "myLoadcell.hpp"
#include "myGpio.hpp"
#include <Arduino.h>

void setForward() {
  digitalWrite(2, HIGH);
}

void setBackward() {
  digitalWrite(2, LOW);
}

void actuate(int speed) {
  analogWrite(3, speed);
}