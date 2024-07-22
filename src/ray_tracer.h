#pragma once

#include "bvh.h"
#include "camera.h"
#include "draw.h"
#include "object.h"
#include "stopwatch.h"

// third party
#include "raylib.h"

// stl
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <unordered_set>
#include <vector>
#include <utility>

inline color lin2srgb(const color& c) {
  auto ret = color(std::pow(c.x(), 1.0 / 2.2), std::pow(c.y(), 1.0 / 2.2),
                   std::pow(c.z(), 1.0 / 2.2));
  ret.clamp(0.0, 1.0);
  return ret;
}

struct ray_tracer {
  enum class render_mode : int {
    color = 0,
    normal,
    depth
  } mode = render_mode::color;

  ray_tracer(class camera cam, size_t sample_count, size_t max_depth,
             size_t image_width, size_t image_height)
      : sample_count(sample_count),
        max_depth(max_depth),
        camera(cam),
        image_width(image_width),
        image_height(image_height) {
    pixel_width = 1.0 / image_width;
    pixel_height = 1.0 / image_height;
    image_linear.resize(image_width * image_height);
  }

  color compute(size_t x, size_t y) {
    vec2 uv = get_uv(x, y);
    color c;
    for (int i = 0; i < sample_count; ++i) {
      vec2 uvp(uv.x() + random_real() * pixel_width, uv.y() + random_real() * pixel_height);
      color sample_color;
      ray r = camera.ray_to(uvp);
      fire_ray(r, sample_color, max_depth);
      c += sample_color;
    }
    c /= sample_count;
    return c;
  }

  void render_pixel(Image& image, size_t x, size_t y) {
    color res;
    switch (mode) {
      case render_mode::color: {
        res = compute(x, y);
        if (accumulate) {
          auto idx = y * image_width + x;
          auto& prev = image_linear[idx];
          res = (prev * (frame_count - 1) + res) / frame_count;
          image_linear[idx] = res;
        }
        res = lin2srgb(res);
        break;
      }
      case render_mode::normal: {
        res = normal_at(x, y);
        break;
      }
      case render_mode::depth: {
        real_t d = depth_at(x, y);
        d = clamp(d, 0.0, camera.focus_distance * 2);
        d /= camera.focus_distance * 2;
        d = 1.0 - d;
        res = color(d);
        break;
      }
    }
    write_pixel(image, x, y, res);
    if (pixels_done.fetch_add(1) == image_width * image_height - 1) {
      ++frame_count;
      pixels_done = 0;
    }
  }

  real_t depth_at(size_t x, size_t y) const {
    hit_record rec;
    if (hit(camera.ray_to(get_uv(x, y)), rec)) {
      return rec.t;
    }
    return -INFINITY;
  }

  vec3 normal_at(size_t x, size_t y) const {
    hit_record rec;
    if (hit(camera.ray_to(get_uv(x, y)), rec)) {
      return rec.normal;
    }
    return vec3(0);
  }

  void set_accumulate(bool accum) {
    if (accumulate == accum)
      return;
    accumulate = accum;
	reset();
  }

  void update_camera(float move_right, float move_front, float look_right,
                     float look_up) {
    if (move_right != 0 || move_front != 0 || look_right != 0 || look_up != 0) {
      reset();
    }
    camera.move(move_right, move_front);
    camera.change_direction(look_right, look_up);
  }

  std::atomic_uint frame_count = 1;
  size_t sample_count;
  size_t max_depth;
  camera camera;
  color background;
  hittable_list world;

 protected:
  void fire_ray(const ray& r, color& c, size_t depth) const {
    if (depth <= 0) {
      c = color(0, 0, 0);
      return;
    }
    hit_record rec = {};
    if (hit(r, rec)) {
      ray scattered;
      color attenuation;
      color emitted = rec.mat->emitted(rec.u, rec.v, rec.p);
      if (rec.mat->scatter(r, rec, attenuation, scattered)) {
        fire_ray(scattered, c, depth - 1);
        c = emitted + attenuation * c;
      } else {
        c = emitted;
      }
    } else {
      c = background;
    }
  }

  bool hit(const ray& r, hit_record& out_rec) const {
    return world.hit(r, 0.001, INFINITY, out_rec);
  }

  void reset() {
    frame_count = 1;
    pixels_done = 0;
    image_linear.assign(image_width * image_height, color(0));
  }

  inline vec2 get_uv(size_t x, size_t y) const {
    return vec2(static_cast<real_t>(x) / (image_width - 1),
                1.0 - static_cast<real_t>(y) / (image_height - 1));
  }

  size_t image_width;
  size_t image_height;
  real_t pixel_width;
  real_t pixel_height;
  bool accumulate = false;
  std::vector<color> image_linear;
  std::atomic_uint pixels_done = 0;
};
