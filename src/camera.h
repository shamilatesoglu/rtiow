#pragma once

#include "ray.h"

class camera {
 public:
  camera(REAL_T hfov, REAL_T aspect_ratio) {
    auto theta = deg2rad(hfov);
    viewport_width = 2.0 * tan(theta / 2);
    viewport_height = viewport_width / aspect_ratio;
  }

  ray ray_to(REAL_T u, REAL_T v) const {
    REAL_T half_width = viewport_width / 2.0;
    REAL_T half_height = viewport_height / 2.0;
    REAL_T x = (u * viewport_width) - half_width;
    REAL_T y = (v * viewport_height) - half_height;

    REAL_T lens_radius = aperture / 2.0;
    vec3 rnd = random_in_unit_disk() * lens_radius;
    vec3 offset = right() * rnd.x() + up() * rnd.y();

    vec3 direction =
      (front * focus_distance) + (right() * x) + (up() * y) - offset;

    return ray(origin + offset, direction);
  }

  vec3 right() const { return front.cross(view_up).normalized(); }

  vec3 up() const { return right().cross(front).normalized(); }

  void look_at(const point3& target) { front = (target - origin).normalized(); }

  void move_forward(REAL_T distance) { origin += front * distance; }

  void move_right(REAL_T distance) { origin += right() * distance; }

  void move(REAL_T right, REAL_T forward) {
    move_right(right);
    move_forward(forward);
  }

  void change_direction(REAL_T du, REAL_T dv) {
    // clang-format off
    REAL_T theta = atan2(front.x(), front.z());
    REAL_T phi = atan2(front.y(), sqrt(front.x() * front.x() + front.z() * front.z()));
    theta += du;
    phi += dv;
    if (phi > M_PI / 2.0 - 0.01) phi = M_PI / 2.0 - 0.01;
    if (phi < -M_PI / 2.0 + 0.01) phi = -M_PI / 2.0 + 0.01;
    front = vec3(sin(theta) * cos(phi), sin(phi), cos(theta) * cos(phi)).normalized();
    // clang-format on
  }

  point3 origin = point3(0.0, 0.5, 0.0);
  vec3 front = vec3(0.0, 0.0, -1.0);
  vec3 view_up = vec3(0.0, 1.0, 0.0);

  float aperture = 0.1;
  float focus_distance = front.length();

 protected:
  REAL_T viewport_height;
  REAL_T viewport_width;
};