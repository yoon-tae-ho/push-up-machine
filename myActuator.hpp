#ifndef MYACTUATOR_H
#define MYACTUATOR_H

#define MAX_ACTUATOR_PWM 255
#define MAX_ACTUATOR_SPEED 11.095 // 11.095 mm/s

class Actuator {
  private:
    int currentPwm;
    bool direction;
    double position; // mm

    bool isAvailable; // power switch
    bool isManualing; // manual switch

    void setDirection();

  public:
    bool isWorking;

    Actuator();

    void setForward();

    void setBackward();

    void actuate(int speed, bool manualing = false);

    double getCurrentPosition();

    void setCurrentPosition(double value);

    void calculatePosition(double timeStep);

    void setAvailable(bool available);

    void setManualing(bool manualing);
};

#endif