#ifndef MYUTILS_HPP
#define MYUTILS_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <EEPROM.h>

void saveValue(int index, double value) {
  EEPROM.write(index, value);
  EEPROM.commit(); // EEPROM에 변경사항을 저장합니다.
}

double loadValue(int index) {
    return EEPROM.read(index);
}

// Moving average filter
std::vector<double> movingAverageFilter(const std::vector<double>& data, int windowSize) {
    std::vector<double> smoothedData(data.size());
    int halfWindow = windowSize / 2;

    for (size_t i = 0; i < data.size(); ++i) {
        double sum = 0.0;
        int count = 0;
        for (int j = -halfWindow; j <= halfWindow; ++j) {
            if (i + j >= 0 && i + j < data.size()) {
                sum += data[i + j];
                count++;
            }
        }
        smoothedData[i] = sum / count;
    }

    return smoothedData;
}

// 극대 극소
void findLocalExtrema(const std::vector<double>& data, std::vector<size_t>& maxima, std::vector<size_t>& minima) {
    for (size_t i = 1; i < data.size() - 1; ++i) {
        if (data[i] > data[i - 1] && data[i] > data[i + 1]) {
            maxima.push_back(i);
        }
        if (data[i] < data[i - 1] && data[i] < data[i + 1]) {
            minima.push_back(i);
        }
    }
}

std::vector<double> getLocalExtremeValue(const std::vector<double>& data, const int& windowSize) {
  // 이동 평균 필터 적용
  std::vector<double> smoothedData = movingAverageFilter(data, windowSize);

  // 극댓값과 극솟값 저장 벡터
  std::vector<size_t> maxima;
  std::vector<size_t> minima;

  // 극값 찾기
  findLocalExtrema(smoothedData, maxima, minima);

  double localMax = 0;
  double localMin = 0;
  int numOfLocalMax = 0;
  int numOfLocalMin = 0;

  // 극댓값 출력
  for (size_t index : maxima) {
      localMax += smoothedData[index];
      ++numOfLocalMax;
  }

  // 극솟값 출력
  for (size_t index : minima) {
      localMin += smoothedData[index];
      ++numOfLocalMin;
  }

  std::vector<double> result;
  result.push_back(localMax / numOfLocalMax);
  result.push_back(localMin / numOfLocalMin);

  return result;
}

#endif