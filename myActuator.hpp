#ifndef MYACTUATOR_H
#define MYACTUATOR_H

#define MAX_ACTUATOR_PWM 255
#define MAX_ACTUATOR_SPEED 4.5 // 4.5 mm/s

class Actuator {
  private:
    int currentPwm;
    bool direction;
    double position; // mm

    void setDirection();

  public:
    bool isWorking;

    Actuator();

    void setForward();

    void setBackward();

    void actuate(int speed);

    double getCurrentPosition();

    void calculatePosition(double timeStep);
};

#endif