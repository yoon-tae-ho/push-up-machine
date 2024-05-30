#ifndef MYACTUATOR_H
#define MYACTUATOR_H

#define MAX_ACTUATOR_PWM_RIGHT 255.0 * (9.987 / 10.345) - 1.0
#define MAX_ACTUATOR_PWM_LEFT 255.0
#define MAX_ACTUATOR_SPEED 9.987 // 9.987 mm/s

#define MAX_ACTUATOR_HEIGHT 210
#define MIN_ACTUATOR_HEIGHT 0

class Actuator {
  private:
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

    void actuate(bool minjae, bool manualing = false);
    
    float getCurrentPosition();

    void setInitialPosition(float value);

    void calculatePosition(float timeStep);

    void setAvailable(bool available);

    void setManualing(bool manualing);

    void checkHeightLimit();
};

#endif