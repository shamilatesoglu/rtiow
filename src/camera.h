#pragma once

#include "ray.h"

#include <optional>

class camera {
 public:
  camera(real_t hfov, real_t aspect_ratio, float aperture = 0.1,
         float focus_distance = 1.0,
         const point3& origin = point3(0.0, 0.5, 0.0),
         real_t shutter_open_time = 0.0, real_t shutter_close_time = 0.0)
      : aperture(aperture),
        focus_distance(focus_distance),
        origin(origin),
        shutter_open_time(shutter_open_time),
        shutter_close_time(shutter_close_time) {
    auto theta = deg2rad(hfov);
    viewport_width = 2.0 * tan(theta / 2);
    viewport_height = viewport_width / aspect_ratio;
  }

  ray ray_to(real_t u, real_t v, bool defocus_blur = true) const {
    real_t half_width = viewport_width / 2.0;
    real_t half_height = viewport_height / 2.0;
    real_t x = (u * viewport_width) - half_width;
    real_t y = (v * viewport_height) - half_height;

    auto hor = right() * focus_distance;
    auto ver = up() * focus_distance;
    real_t lens_radius = aperture / 2.0;
    vec3 rnd =
      defocus_blur ? random_in_unit_disk() * lens_radius : vec3(0, 0, 0);
    vec3 offset = hor * rnd.x() + ver * rnd.y();

    vec3 direction = (front * focus_distance) + (hor * x) + (ver * y) - offset;

    return ray(origin + offset, direction,
               random_real(shutter_open_time, shutter_close_time));
  }

  /// Project a 3D point to the view space.
  /// Returns the point in the range [-viewport_width/2, viewport_width/2] x [-viewport_height/2, viewport_height/2]
  /// Returns nullopt if the point is behind the camera.
  std::optional<vec2> project(const vec3& world_point) const {
    vec3 c2p = world_point - origin;
    real_t d = c2p.dot(front);
    if (d <= 0) {
      return std::nullopt;
    }
    real_t x = c2p.dot(right()) / d;
    real_t y = c2p.dot(up()) / d;
    // Correct the image w.r.t. viewport size & FOV
    x = x / viewport_width * 2.0;
    y = y / viewport_height * 2.0;
    return vec2(x, y);
  }

  std::optional<vec2> screen_space(const vec3& world_point,
                                   const vec2& size) const {
    auto p = project(world_point);
    if (!p) {
      return std::nullopt;
    }
    // Correct the image w.r.t. viewport size & FOV
    p->x() = (p->x() + 1.0) / 2.0 * size.x();
    p->y() = (1.0 - p->y()) / 2.0 * size.y();
    return p;
  }

  vec3 right() const { return front.cross(view_up).normalized(); }

  vec3 up() const { return right().cross(front).normalized(); }

  void look_at(const point3& target) { front = (target - origin).normalized(); }

  void move_forward(real_t distance) { origin += front * distance; }

  void move_right(real_t distance) { origin += right() * distance; }

  void move(real_t right, real_t forward) {
    move_right(right);
    move_forward(forward);
  }

  void change_direction(real_t du, real_t dv) {
    // clang-format off
    real_t theta = atan2(front.x(), front.z());
    real_t phi = atan2(front.y(), sqrt(front.x() * front.x() + front.z() * front.z()));
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

  real_t shutter_open_time = 0.0;
  real_t shutter_close_time = 0.0;

 protected:
  real_t viewport_height;
  real_t viewport_width;
};
