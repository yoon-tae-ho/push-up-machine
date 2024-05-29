#ifndef MYUSER_H
#define MYUSER_H

class User {
  private:
    double maxRefLoad;
    double minRefLoad;
    double realMax;
    double realMin;

    int state; // (0 ~ 3 : up ~ down)
    int prevState;

    double refDiff;

  public:
    bool wentDown;
    bool autoMode;

    int totalCount;
    int levelCount;

    User();

    void setRefLoad(const double& max, const double& min, const double& currentPos);

    double getMaxRefLoad();

    double getMinRefLoad();

    double getMidRefLoad();

    void setState(const int& newState);

    int getState();

    int getPrevState();

    void calculateRef(const double& currentPos);
};

#endif