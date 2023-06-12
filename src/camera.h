#pragma once

#include "ray.h"

class camera {
 public:
  camera(REAL_T hfov, REAL_T aspect_ratio) {
    auto theta = deg2rad(hfov);
    viewport_width = 2.0 * tan(theta / 2);
    viewport_height = viewport_width / aspect_ratio;
    update();
  }

  ray get_ray(REAL_T u, REAL_T v) const {
    return ray(origin,
               lower_left_corner + u * horizontal + v * vertical - origin);
  }

  void look_at(const point3& target) {
    front = (target - origin).normalized();
    update();
  }

  void move(vec3 direction, REAL_T distance) { origin += distance * direction; }

  point3 origin = point3(0, 0, 0);
  vec3 front;

  void update() {
    auto focal_length = 1.0;
    horizontal = vec3(viewport_width, 0.0, 0.0);
    vertical = vec3(0.0, viewport_height, 0.0);
    lower_left_corner =
        origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);
  }

 protected:
  REAL_T viewport_height;
  REAL_T viewport_width;
  point3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;
};