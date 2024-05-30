#include "myUtils.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <EEPROM.h>

void saveValue(int index, float value) {
  EEPROM.write(index, value);
  EEPROM.commit(); // EEPROM에 변경사항을 저장합니다.
}

float loadValue(int index) {
    return EEPROM.read(index);
}

// Moving average filter
std::vector<float> movingAverageFilter(const std::vector<float>& data, int windowSize) {
    std::vector<float> smoothedData(data.size());
    int halfWindow = windowSize / 2;

    for (size_t i = 0; i < data.size(); ++i) {
        float sum = 0.0;
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
void findLocalExtrema(const std::vector<float>& data, std::vector<size_t>& maxima, std::vector<size_t>& minima) {
    for (size_t i = 1; i < data.size() - 1; ++i) {
        if (data[i] > data[i - 1] && data[i] > data[i + 1]) {
            maxima.push_back(i);
        }
        if (data[i] < data[i - 1] && data[i] < data[i + 1]) {
            minima.push_back(i);
        }
    }
}

std::vector<float> getLocalExtremeValue(const std::vector<float>& data, const int& windowSize) {
  // 이동 평균 필터 적용
  std::vector<float> smoothedData = movingAverageFilter(data, windowSize);

  // 극댓값과 극솟값 저장 벡터
  std::vector<size_t> maxima;
  std::vector<size_t> minima;

  // 극값 찾기
  findLocalExtrema(smoothedData, maxima, minima);

  float localMax = 0;
  float localMin = 0;
  int numOfLocalMax = 0;
  int numOfLocalMin = 0;

  Serial.println("============================================");
  // 극댓값 출력
  for (size_t index : maxima) {
      if (fabs(smoothedData[index]) < 10) continue;
      localMax += smoothedData[index];
      ++numOfLocalMax;
      Serial.print(smoothedData[index]);
      Serial.print(", ");
  }

  Serial.println("--------------------------------------------");
  // 극솟값 출력
  for (size_t index : minima) {
      if (fabs(smoothedData[index]) < 10) continue;
      localMin += smoothedData[index];
      ++numOfLocalMin;
      Serial.print(smoothedData[index]);
      Serial.print(", ");
  }
  Serial.println("============================================");

  std::vector<float> result;
  result.push_back(localMax / numOfLocalMax);
  result.push_back(localMin / numOfLocalMin);

  return result;
}
