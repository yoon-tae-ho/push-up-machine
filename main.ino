#include "HX711.h"
#include "myHeader.hpp"
#include "myGpio.hpp"

// Setting for Loadcell
HX711 scale_right(DOUT_RIGHT, CLK_RIGHT);
HX711 scale_left(DOUT_LEFT, CLK_LEFT);

// Constants
int startTime = 0;
int checkTime = 0;
float duringTime = 0;

// Constants for Loadcell
double load_right = 0.0;
double load_left = 0.0;

// Constants for Actuator
int count = 0;
double referenceValue = 10.0;
bool isOver = false;

void setup() {
  Serial.begin(115200);  // 값 모니터링 위해서...
  startTime = millis();

  // Loadcell
  scale_right.set_scale(calibration_factor); 
  scale_right.tare();   //영점잡기. 현재 측정값을 0으로 둔다.
  scale_left.set_scale(calibration_factor); 
  scale_left.tare();   //영점잡기. 현재 측정값을 0으로 둔다.

  // Actuator
  pinMode(DIR_RIGHT, OUTPUT); // Configure pin 7 as an Output
  pinMode(PWM_RIGHT, OUTPUT); // Configure pin 7 as an Output
  pinMode(DIR_LEFT, OUTPUT); // Configure pin 7 as an Output
  pinMode(PWM_LEFT, OUTPUT); // Configure pin 8 as an Output

  // Switch
  pinMode(POWER, INPUT_PULLUP); // Configure pin 7 as an Output
  pinMode(MANUAL_ACTUATOR_UP, INPUT_PULLUP); // Configure pin 7 as an Output
  pinMode(MANUAL_ACTUATOR_DOWN, INPUT_PULLUP); // Configure pin 7 as an Output
  pinMode(ZERO_ADJUSTMENT, INPUT_PULLUP); // Configure pin 8 as an Output
}

void loop() {
  checkTime = millis();
  duringTime = (float)(checkTime - startTime) / 1000.0;

  // Loadcell
  load_right = (double)(scale_right.get_units());
  load_left = (double)(scale_left.get_units());
  Serial.print("Right: ");
  Serial.println(load_right);
  Serial.print("Left: ");
  Serial.println(load_left);
  Serial.println("======================================");

  // Actuator
  if (load_left > referenceValue) {
    isOver = true;
  } else {
    if (isOver) {
        ++count;
    }
    isOver = false;
  }

  if (count >= 3) {
    setForward();
    actuate(MAX_ACTUATOR_SPEED);
    delay(5000);
    actuate(0);
    count = 0;
  }
}