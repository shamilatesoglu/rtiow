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

inline int fastrand() {
  static unsigned int g_seed = 42;
  g_seed = (214013 * g_seed + 2531011);
  return (g_seed >> 16) & 0x7FFF;
}

inline REAL_T random_real() {
  return static_cast<REAL_T>(fastrand()) / 0x7FFF;
}

inline REAL_T random_real(REAL_T min, REAL_T max) {
  return min + (max - min) * random_real();
}

inline int random_int(int min, int max) {
  return fastrand() % (max - min + 1) + min;
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