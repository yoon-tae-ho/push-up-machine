#ifndef MYACTUATOR_H
#define MYACTUATOR_H

#define MAX_ACTUATOR_PWM 255
#define MAX_ACTUATOR_SPEED 11.095 // 11.095 mm/s

class Actuator {
  private:
    int currentPwm;
    double position; // mm

    bool isAvailable; // power switch
    bool isManualing; // manual switch

    void setDirection();

  public:
    bool isWorking;
    bool direction;

    Actuator();

    void setForward();

    void setBackward();

    void actuate(int speed, bool manualing = false);

    double getCurrentPosition();

    void setInitialPosition(double value);

    void calculatePosition(double timeStep);

    void setAvailable(bool available);

    void setManualing(bool manualing);
};

#endif