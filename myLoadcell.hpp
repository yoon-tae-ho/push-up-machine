#ifndef MYLOADCELL_H
#define MYLOADCELL_H

#define calibration_factor 7050.0

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
};

#endif