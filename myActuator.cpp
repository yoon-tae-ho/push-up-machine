// myActuator.c
#include <Arduino.h>
#include "myActuator.hpp"
#include "myGpio.hpp"
#include "myUtils.hpp"

// private

void Actuator::setDirection() {
  digitalWrite(DIR_RIGHT, direction);
  digitalWrite(DIR_LEFT, direction);
}

// public

Actuator::Actuator() {
  pinMode(DIR_RIGHT, OUTPUT);
  pinMode(PWM_RIGHT, OUTPUT);
  pinMode(DIR_LEFT, OUTPUT);
  pinMode(PWM_LEFT, OUTPUT);

  currentPwm = 0;
  position = 0;
  direction = HIGH;
  isWorking = false;
  isAvailable = false;
  isManualing = false;
}

void Actuator::setForward() {
  direction = HIGH;
  setDirection();
}

void Actuator::setBackward() {
  direction = LOW;
  setDirection();
}

void Actuator::actuate(int pwm, bool manualing) {
  if (!isAvailable) return;

  if (isManualing && !manualing) return;

  analogWrite(PWM_RIGHT, pwm);
  analogWrite(PWM_LEFT, pwm);

  isWorking = (pwm != 0);
  currentPwm = pwm;
}

float Actuator::getCurrentPosition() {
  return position;
}

void Actuator::setInitialPosition(float value) {
  position = value;
}

void Actuator::calculatePosition(float timeStep) {
  if (!isWorking) return;

  float displacement = (float)MAX_ACTUATOR_SPEED * timeStep; // v (mm/s) * t (s)
  if (direction == LOW) displacement *= -1;
  position += displacement;

  if (position < 0) position = 0;
  else if (position > 300.0) position = 300.0;

  // save at flash memory
  saveValue(EEPROM_POSITION_INDEX, position);
}

void Actuator::setAvailable(bool available) {
  if (!available) actuate(0, true);
  isAvailable = available;
}

void Actuator::setManualing(bool manualing) {
  isManualing = manualing;
}