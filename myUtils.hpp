#ifndef MYUTILS_HPP
#define MYUTILS_HPP

#include <vector>

void saveValue(int index, float value);
float loadValue(int index);
std::vector<float> movingAverageFilter(const std::vector<float>& data, int windowSize);
void findLocalExtrema(const std::vector<float>& data, std::vector<size_t>& maxima, std::vector<size_t>& minima);
std::vector<float> getLocalExtremeValue(const std::vector<float>& data, const int& windowSize);

#endif