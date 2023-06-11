#pragma once

#include "common.h"

#include <cmath>
#include <ostream>

struct vec3 {
  vec3() : e{0, 0, 0} {}
  vec3(REAL_T e0, REAL_T e1, REAL_T e2) : e{e0, e1, e2} {}

  REAL_T x() const { return e[0]; }
  REAL_T y() const { return e[1]; }
  REAL_T z() const { return e[2]; }

  vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
  REAL_T operator[](int i) const { return e[i]; }
  REAL_T& operator[](int i) { return e[i]; }

  vec3& operator+=(const vec3& v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
  }

  vec3& operator*=(const REAL_T t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  vec3& operator/=(const REAL_T t) { return *this *= 1 / t; }

  REAL_T length() const { return sqrt(length_squared()); }

  REAL_T length_squared() const {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }

  REAL_T e[3];
};  // vec3 Utility Functions

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
  return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
  return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
  return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
  return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(REAL_T t, const vec3& v) {
  return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3& v, REAL_T t) {
  return t * v;
}

inline vec3 operator/(vec3 v, REAL_T t) {
  return (1 / t) * v;
}

inline REAL_T dot(const vec3& u, const vec3& v) {
  return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
  return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
              u.e[2] * v.e[0] - u.e[0] * v.e[2],
              u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(vec3 v) {
  return v / v.length();
}

using point3 = vec3;   // 3D point
using color = vec3;    // RGB color