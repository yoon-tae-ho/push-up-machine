#ifndef MYACTUATOR_H
#define MYACTUATOR_H

#define MAX_ACTUATOR_PWM 255
#define MAX_ACTUATOR_SPEED 11.095 // 11.095 mm/s

class Actuator {
  private:
    int currentPwm;
    float position; // mm

    bool isManualing; // manual switch

    void setDirection();

  public:
    bool isAvailable; // power switch
    bool isWorking;
    bool direction;

    Actuator();

    void setForward();

    void setBackward();

    void actuate(int speed, bool manualing = false);

    float getCurrentPosition();

    void setInitialPosition(float value);

    void calculatePosition(float timeStep);

    void setAvailable(bool available);

    void setManualing(bool manualing);
};

#endif