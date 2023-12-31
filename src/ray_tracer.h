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
    hit_recs.resize(image_width * image_height);
  }

  color compute(real_t u, real_t v) {
    color c;
    for (int i = 0; i < sample_count; ++i) {
      real_t up = u + random_real() * pixel_width;
      real_t vp = v + random_real() * pixel_height;
      color sample_color;
      ray r = camera.ray_to(up, vp);
      fire_ray(r, sample_color, max_depth);
      c += sample_color;
    }
    c /= sample_count;
    c.x() = pow(c.x(), 1.0 / 2.2);
    c.y() = pow(c.y(), 1.0 / 2.2);
    c.z() = pow(c.z(), 1.0 / 2.2);
    c.clamp(0.0, 1.0);
    return c;
  }

  void render_pixel(Image& image, size_t x, size_t y) {
    real_t u = static_cast<real_t>(x) / (image_width - 1);
    real_t v = static_cast<real_t>(y) / (image_height - 1);
    v = 1.0 - v;
    color c = compute(u, v);
    switch (mode) {
      case render_mode::color:
        write_pixel(image, x, y, c);
        break;
      case render_mode::normal:
        write_pixel(image, x, y, normal_at(x, y));
        break;
      case render_mode::depth:
        real_t d = depth_at(x, y);
        d = clamp(d, 0.0, camera.focus_distance * 2);
        d /= camera.focus_distance * 2;
        d = 1.0 - d;
        write_pixel(image, x, y, d);
        break;
    }
  }

  real_t depth_at(size_t x, size_t y) const {
    size_t idx = y * image_width + x;
    if (idx >= hit_recs.size()) {
      return -INFINITY;
    }
    return hit_recs[idx].t;
  }

  vec3 normal_at(size_t x, size_t y) const {
    auto& rec = hit_recs[y * image_width + x];
    return rec.normal;
  }

  size_t sample_count;
  size_t max_depth;
  camera camera;
  color background;
  hittable_list world;

 protected:
  void fire_ray(const ray& r, color& c, size_t depth) {
    if (depth <= 0) {
      c = color(0, 0, 0);
      return;
    }
    hit_record rec = {};
    if (world.hit(r, 0.001, INFINITY, rec)) {
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
    if (auto ss = camera.screen_space(r.origin() + r.direction(),
                                      vec2(image_width, image_height))) {
      auto x = static_cast<size_t>(ss->x());
      auto y = static_cast<size_t>(ss->y());
      if (x >= 0 && x < image_width && y >= 0 && y < image_height)
        hit_recs[y * image_width + x] = rec;
    }
  }

  std::vector<hit_record> hit_recs;

  size_t image_width;
  size_t image_height;
  real_t pixel_width;
  real_t pixel_height;
};
