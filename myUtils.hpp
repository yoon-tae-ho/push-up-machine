#ifndef MYUTILS_HPP
#define MYUTILS_HPP

// #include <iostream>
#include <vector>

void saveValue(int index, double value);
double loadValue(int index);
std::vector<double> movingAverageFilter(const std::vector<double>& data, int windowSize);
void findLocalExtrema(const std::vector<double>& data, std::vector<size_t>& maxima, std::vector<size_t>& minima);
std::vector<double> getLocalExtremeValue(const std::vector<double>& data, const int& windowSize);

#endif