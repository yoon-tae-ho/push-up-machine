#ifndef MYUTILS_HPP
#define MYUTILS_HPP

#include <EEPROM.h>

void saveValue(int index, double value) {
  EEPROM.write(index, value);
  EEPROM.commit(); // EEPROM에 변경사항을 저장합니다.
}

double loadValue(int index) {
  return EEPROM.read(index);
}

#endif