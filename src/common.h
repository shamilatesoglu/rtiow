#pragma once

#ifndef real_t
#define real_t double
#endif

#ifdef _WIN32
#define M_PI 3.14159265358979323846
#endif
#include <cmath>
#include <memory>
#include <random>

inline real_t deg2rad(real_t degrees) {
  return degrees * M_PI / 180.0;
}

inline int fastrand() {
  static unsigned int g_seed = 42;
  g_seed = (214013 * g_seed + 2531011);
  return (g_seed >> 16) & 0x7FFF;
}

inline real_t random_real() {
  return static_cast<real_t>(fastrand()) / 0x7FFF;
}

inline real_t random_real(real_t min, real_t max) {
  return min + (max - min) * random_real();
}

inline int random_int(int min, int max) {
  return fastrand() % (max - min + 1) + min;
}

inline real_t clamp(real_t x, real_t min, real_t max) {
  if (x < min) {
    return min;
  }
  if (x > max) {
    return max;
  }
  return x;
}

inline real_t lerp(real_t t, real_t a, real_t b) {
  return (1.0 - t) * a + t * b;
}
