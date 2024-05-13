// myActuator.c
#include "myActuator.hpp"
#include "myGpio.hpp"
#include <Arduino.h>

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

double Actuator::getCurrentPosition() {
  return position;
}

void Actuator::calculatePosition(double timeStep) {
  if (!isWorking) return;

  double displacement = (double)MAX_ACTUATOR_SPEED * timeStep; // v (mm/s) * t (s)
  if (direction == LOW) displacement *= -1;
  position += displacement;
}

void Actuator::setAvailable(bool available) {
  if (!available) actuate(0, true);
  isAvailable = available;
}

void Actuator::setManualing(bool manualing) {
  isManualing = manualing;
}