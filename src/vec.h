#pragma once

#include "common.h"

#include <cmath>
#include <ostream>

struct vec3 {
  vec3() : e{0, 0, 0} {}

  vec3(real_t e0, real_t e1, real_t e2) : e{e0, e1, e2} {}

  real_t x() const { return e[0]; }

  real_t y() const { return e[1]; }

  real_t z() const { return e[2]; }

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

  vec3 operator+(real_t s) const { return vec3(e[0] + s, e[1] + s, e[2] + s); }

  vec3 operator-(real_t s) const { return vec3(e[0] - s, e[1] - s, e[2] - s); }

  vec3 operator*(real_t t) const { return vec3(t * e[0], t * e[1], t * e[2]); }

  vec3 operator/(real_t t) const { return vec3(e[0] / t, e[1] / t, e[2] / t); }

  vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }

  vec3& operator+=(const vec3& v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
  }

  vec3& operator+=(real_t s) {
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

  vec3& operator*=(const real_t t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  vec3& operator*=(const vec3& v) {
    e[0] *= v[0];
    e[1] *= v[1];
    e[2] *= v[2];
    return *this;
  }

  vec3 rotated(const vec3& axis, real_t angle) const {
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

  real_t dot(const vec3& rhs) const {
    return e[0] * rhs.e[0] + e[1] * rhs.e[1] + e[2] * rhs.e[2];
  }

  vec3& operator/=(const real_t t) { return *this *= 1 / t; }

  real_t length() const { return sqrt(length_squared()); }

  real_t length_squared() const {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }

  vec3 normalized() const {
    real_t len = length();
    return vec3(e[0] / len, e[1] / len, e[2] / len);
  }

  bool near_zero() const {
    // Return true if the vector is close to zero in all dimensions.
    const auto s = 1e-8;
    return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
  }

  static vec3 random();
  static vec3 random(real_t min, real_t max);

  real_t operator[](int i) const { return e[i]; }

  real_t& operator[](int i) { return e[i]; }

  real_t e[3];
};  // vec3 Utility Functions

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
  return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator*(real_t t, const vec3& v) {
  return v * t;
}

inline vec3 reflect(const vec3& in, const vec3& normal) {
  return in - 2 * in.dot(normal) * normal;
}

inline vec3 refract(const vec3& in, const vec3& normal, real_t etai_over_etat) {
  real_t cos_theta = fmin(-in.dot(normal), 1.0);
  vec3 r_out_perp = etai_over_etat * (in + cos_theta * normal);
  vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * normal;
  return r_out_perp + r_out_parallel;
}

inline real_t reflectance(real_t cosine, real_t ref_idx) {
  // Use Schlick's approximation for reflectance.
  auto r0 = (1 - ref_idx) / (1 + ref_idx);
  r0 = r0 * r0;
  return r0 + (1 - r0) * pow((1 - cosine), 5);
}

inline vec3 vec3::random(real_t min, real_t max) {
  return vec3(random_real(min, max), random_real(min, max),
              random_real(min, max));
}

inline vec3 vec3::random() {
  return vec3(random_real(), random_real(), random_real());
}

inline vec3 random_unit_vector() {
  auto a = random_real() * 2 * M_PI;
  auto z = random_real() * 2 - 1;
  auto r = sqrt(1 - z * z);
  return vec3(r * cos(a), r * sin(a), z);
}

inline vec3 random_in_unit_disk() {
  auto a = random_real() * 2 * M_PI;
  auto r = sqrt(random_real());
  return vec3(r * cos(a), r * sin(a), 0);
}

using point3 = vec3;  // 3D point
using color = vec3;   // RGB color

struct aabb {
  aabb() {}

  aabb(const point3& a, const point3& b) : min(a), max(b) {}

  bool overlaps(const aabb& other) const {
    return (min.x() <= other.max.x() && max.x() >= other.min.x()) &&
           (min.y() <= other.max.y() && max.y() >= other.min.y()) &&
           (min.z() <= other.max.z() && max.z() >= other.min.z());
  }

  point3 min;
  point3 max;
};
