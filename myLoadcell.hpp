#ifndef MYLOADCELL_H
#define MYLOADCELL_H

#define calibration_factor 7050.0

class Loadcell {
  private:
    HX711 scaleRight;
    HX711 scaleLeft;

  public:

    Loadcell();

    double getRightLoad();

    double getLeftLoad();
};

#endif