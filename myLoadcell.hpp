#ifndef MYLOADCELL_H
#define MYLOADCELL_H

#include "HX711.h"

#define calibration_factor 14500

class Loadcell {
  private:
    HX711 scaleRight;
    HX711 scaleLeft;

    float balanceFactor; // 0 ~ 1

  public:
    bool isBalanced;

    Loadcell();

    double getRightLoad();

    double getLeftLoad();

    bool checkBalance();

    bool checkBalanceDirection();
};

#endif