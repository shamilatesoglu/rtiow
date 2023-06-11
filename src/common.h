#pragma once

#ifndef REAL_T
#define REAL_T double
#endif

#include <cmath>
#include <random>

inline REAL_T deg2rad(REAL_T degrees) { return degrees * M_PI / 180.0; }

inline REAL_T random_real() {
  static std::uniform_real_distribution<REAL_T> distribution(0.0, 1.0);
  static std::mt19937 generator;
  return distribution(generator);
}

inline REAL_T clamp(REAL_T x, REAL_T min, REAL_T max) {
  if (x < min) {
    return min;
  }
  if (x > max) {
    return max;
  }
  return x;
}