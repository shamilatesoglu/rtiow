#include "object.h"

#include <cfloat>
#include <mutex>
#include <unordered_set>

void get_sphere_uv(const vec3& p, real_t& u, real_t& v) {
  // p: point on unit sphere
  // u: [0, 1] longitude
  // v: [0, 1] latitude

  auto theta = acos(-p.y());
  auto phi = atan2(-p.z(), p.x()) + M_PI;
  u = phi / (2 * M_PI);
  v = theta / M_PI;
}

void get_plane_uv(const vec3& p, real_t& u, real_t& v) {
  u = p.x();
  v = p.z();
}

bool sphere::hit(const ray& r, real_t t_min, real_t t_max,
                 hit_record& rec) const {
  // Ray Center: R
  // Sphere Center: S
  // Day Direction: d
  // Ray Eq: R(t) = R + td
  // ((t^2)d ⋅ d) + (2td ⋅ (R−S)) + ((R−S) ⋅ (R−S)) − r^2 = 0
  const auto& R = r.origin();
  const auto& S = center;
  const auto& d = r.direction();
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
  get_sphere_uv(outward_normal, rec.u, rec.v);
  rec.mat = mat;
  return true;
}

bool sphere::bounding_box(real_t time0, real_t time1, aabb& out) const {
  out = aabb(center - vec3(radius, radius, radius),
             center + vec3(radius, radius, radius));
  return true;
}

bool moving_sphere::hit(const ray& r, real_t t_min, real_t t_max,
                        hit_record& rec) const {
  // Ray Center: R
  // Sphere Center: S
  // Day Direction: d
  // Ray Eq: R(t) = R + td
  // ((t^2)d ⋅ d) + (2td ⋅ (R−S)) + ((R−S) ⋅ (R−S)) − r^2 = 0
  const auto& R = r.origin();
  const auto& S = center(r.time());
  const auto& d = r.direction();
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
  get_sphere_uv(outward_normal, rec.u, rec.v);
  rec.mat = mat;
  return true;
}

bool moving_sphere::bounding_box(real_t time0, real_t time1, aabb& out) const {
  out = (aabb(center(time0) - vec3(radius, radius, radius),
              center(time0) + vec3(radius, radius, radius))
           .surrounding(aabb(center(time1) - vec3(radius, radius, radius),
                             center(time1) + vec3(radius, radius, radius))));
  return true;
}

bool plane::hit(const ray& r, real_t t_min, real_t t_max,
                hit_record& rec) const {
  // Ray Center: R
  // Plane Center: S
  // Day Direction: d
  // Ray Eq: R(t) = R + td
  // (S - R) ⋅ n = 0
  // (S - (R + td)) ⋅ n = 0
  // (S - R) ⋅ n - t(d ⋅ n) = 0
  // t = ((S - R) ⋅ n) / (d ⋅ n)
  const auto& R = r.origin();
  const auto& S = center;
  const auto& d = r.direction();
  auto n = normal;
  auto SR = S - R;
  auto t = SR.dot(n) / d.dot(n);
  if (t < t_min || t > t_max) {
    return false;
  }
  rec.t = t;
  rec.p = r.at(t);
  rec.set_face_normal(r, n);
  get_plane_uv(rec.p, rec.u, rec.v);
  rec.mat = mat;
  return true;
}

bool plane::bounding_box(real_t time0, real_t time1, aabb& out) const {
  return false;
}