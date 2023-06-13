#pragma once

#ifndef REAL_T
#define REAL_T double
#endif

#ifdef _WIN32
#define M_PI 3.14159265358979323846
#endif
#include <cmath>
#include <random>
#include <memory>

#include "vec.h"

inline REAL_T deg2rad(REAL_T degrees) {
  return degrees * M_PI / 180.0;
}

inline REAL_T random_real() {
  static std::uniform_real_distribution<REAL_T> distribution(0.0, 1.0);
  static std::seed_seq seed{42};
  static std::mt19937 generator(seed);
  return distribution(generator);
}

inline REAL_T random_real(REAL_T min, REAL_T max) {
  return min + (max - min) * random_real();
}

inline int random_int(int min, int max) {
  return static_cast<int>(random_real(min, max + 1));
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

inline vec3 random_unit_vector() {
  auto a = random_real() * 2 * M_PI;
  auto z = random_real() * 2 - 1;
  auto r = sqrt(1 - z * z);
  return vec3(r * cos(a), r * sin(a), z);
}