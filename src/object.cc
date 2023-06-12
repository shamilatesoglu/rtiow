#include "object.h"

bool sphere::hit(const ray& r,
                 REAL_T t_min,
                 REAL_T t_max,
                 hit_record& rec) const {
  // Ray Center: R
  // Sphere Center: S
  // Day Direction: d
  // Ray Eq: R(t) = R + td
  // ((t^2)d ⋅ d) + (2td ⋅ (R−S)) + ((R−S) ⋅ (R−S)) − r^2 = 0
  auto const& R = r.origin();
  auto const& S = center;
  auto const& d = r.direction();
  // D: From sphere center to ray origin
  auto SR = R - S;
  // At^2 + Bt + C = 0, solve for t
  auto A = d.dot(d);
  auto B = 2 * d.dot(SR);
  auto C = SR.dot(SR) - radius * radius;
  auto discriminant = B * B - 4 * A * C;
  if (discriminant < 0) {
    return false;
  }
  auto t = (-B - sqrt(discriminant)) / (2 * A);
  if (t < t_min || t > t_max) {
    return false;
  }
  rec.t = t;
  rec.p = r.at(t);
  vec3 outward_normal = (rec.p - S) / radius;
  rec.set_face_normal(r, outward_normal);
  rec.mat = mat;
  return true;
}

bool plane::hit(const ray& r,
                REAL_T t_min,
                REAL_T t_max,
                hit_record& rec) const {
  // Ray Center: R
  // Plane Center: S
  // Day Direction: d
  // Ray Eq: R(t) = R + td
  // (S - R) ⋅ n = 0
  // (S - (R + td)) ⋅ n = 0
  // (S - R) ⋅ n - t(d ⋅ n) = 0
  // t = ((S - R) ⋅ n) / (d ⋅ n)
  auto const& R = r.origin();
  auto const& S = center;
  auto const& d = r.direction();
  auto n = normal;
  auto SR = S - R;
  auto t = SR.dot(n) / d.dot(n);
  if (t < t_min || t > t_max) {
    return false;
  }
  rec.t = t;
  rec.p = r.at(t);
  rec.set_face_normal(r, n);
  rec.mat = mat;
  return true;
}