#ifndef MYUSER_H
#define MYUSER_H

class User {
  private:
    double maxRefLoad;
    double minRefLoad;

    int state; // (0 ~ 3 : up ~ down)
    int prevState;
    int count;

  public:
    bool wentDown;
    
    User();

    void setRefLoad(const double& max, const double& min);

    double getMaxRefLoad();

    double getMinRefLoad();

    double getMidRefLoad();

    void setState(const int& newState);

    int getState();

    int getPrevState();

    void setCount(const int& newCount);

    int getCount();
};

#endif