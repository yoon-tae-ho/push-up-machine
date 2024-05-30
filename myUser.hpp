#ifndef MYUSER_H
#define MYUSER_H

class User {
  private:
    float maxRefLoad;
    float minRefLoad;
    float realMax;
    float realMin;

    int state; // (0 ~ 3 : up ~ down)
    int prevState;
    int pprevState;

    float refDiff;

  public:
    bool wentDown;
    bool autoMode;

    int totalCount;
    int levelCount;

    User();

    void setRefLoad(const float& max, const float& min, const float& currentPos);

    float getMaxRefLoad();

    float getMinRefLoad();

    float getMidRefLoad();

    void setState(const int& newState);

    int getState();

    int getPrevState();

    int getPprevState();

    void calculateRef(const float& currentPos);
};

#endif