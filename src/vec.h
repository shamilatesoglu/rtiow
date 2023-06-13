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

  vec3 operator+(const vec3& rhs) const {
    return vec3(e[0] + rhs.e[0], e[1] + rhs.e[1], e[2] + rhs.e[2]);
  }
  vec3 operator-(const vec3& rhs) const {
    return vec3(e[0] - rhs.e[0], e[1] - rhs.e[1], e[2] - rhs.e[2]);
  }
  vec3 operator*(const vec3& rhs) const {
    return vec3(e[0] * rhs.e[0], e[1] * rhs.e[1], e[2] * rhs.e[2]);
  }
  vec3 operator/(const vec3& rhs) const {
    return vec3(e[0] / rhs.e[0], e[1] / rhs.e[1], e[2] / rhs.e[2]);
  }
  vec3 operator+(REAL_T s) const { return vec3(e[0] + s, e[1] + s, e[2] + s); }
  vec3 operator-(REAL_T s) const { return vec3(e[0] - s, e[1] - s, e[2] - s); }
  vec3 operator*(REAL_T t) const { return vec3(t * e[0], t * e[1], t * e[2]); }
  vec3 operator/(REAL_T t) const { return vec3(e[0] / t, e[1] / t, e[2] / t); }
  vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }

  vec3& operator+=(const vec3& v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
  }

  vec3& operator+=(REAL_T s) {
    e[0] += s;
    e[1] += s;
    e[2] += s;
    return *this;
  }

  vec3& operator-=(const vec3& v) {
    e[0] -= v.e[0];
    e[1] -= v.e[1];
    e[2] -= v.e[2];
    return *this;
  }

  vec3& operator*=(const REAL_T t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  vec3 rotated(const vec3& axis, REAL_T angle) const {
    // Rodrigues' rotation formula
    // https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
    return *this * cos(angle) + axis.cross(*this) * sin(angle) +
           axis * axis.dot(*this) * (1 - cos(angle));
  }

  vec3 cross(const vec3& rhs) const {
    return vec3(e[1] * rhs.e[2] - e[2] * rhs.e[1],
                e[2] * rhs.e[0] - e[0] * rhs.e[2],
                e[0] * rhs.e[1] - e[1] * rhs.e[0]);
  }

  REAL_T dot(const vec3& rhs) const {
    return e[0] * rhs.e[0] + e[1] * rhs.e[1] + e[2] * rhs.e[2];
  }

  vec3& operator/=(const REAL_T t) { return *this *= 1 / t; }

  REAL_T length() const { return sqrt(length_squared()); }

  REAL_T length_squared() const {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }

  vec3 normalized() const {
    REAL_T len = length();
    return vec3(e[0] / len, e[1] / len, e[2] / len);
  }

  bool near_zero() const {
    // Return true if the vector is close to zero in all dimensions.
    const auto s = 1e-8;
    return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
  }

  REAL_T operator[](int i) const { return e[i]; }
  REAL_T& operator[](int i) { return e[i]; }

  REAL_T e[3];
};  // vec3 Utility Functions

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
  return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator*(REAL_T t, const vec3& v) {
  return v * t;
}

inline vec3 reflect(const vec3& in, const vec3& normal) {
  return in - 2 * in.dot(normal) * normal;
}

inline vec3 refract(const vec3& in, const vec3& normal, REAL_T etai_over_etat) {
  REAL_T cos_theta = fmin(-in.dot(normal), 1.0);
  vec3 r_out_perp = etai_over_etat * (in + cos_theta * normal);
  vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * normal;
  return r_out_perp + r_out_parallel;
}

using point3 = vec3;  // 3D point
using color = vec3;   // RGB color