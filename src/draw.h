#pragma once

#include "vec.h"

// third party
#include "raylib.h"

inline color real_to_screen(const color& c) {
  return 255.999 * c;
}

inline color read_pixel(const class Image& image, size_t x, size_t y) {
  Color c = GetImageColor(image, x, y);
  return color(c.r, c.g, c.b) / 255.999;
}

inline void write_pixel(Image& image, size_t x, size_t y, const color& c) {
  auto sc = real_to_screen(c);
  auto ir = static_cast<uint8_t>(sc.x());
  auto ig = static_cast<uint8_t>(sc.y());
  auto ib = static_cast<uint8_t>(sc.z());
  Color color = {ir, ig, ib, 255};
  ImageDrawPixel(&image, x, y, color);
}

void write_pixel_with_depth_test(Image& image, size_t x, size_t y,
                                 const color& c, bool depth_test, real_t depth,
                                 const class ray_tracer& tracer);

void draw_line(Image& img, const point2& p0, const point2& p1, const color& c);
void draw_aabb(Image& img, const struct aabb& box,
               const class ray_tracer& tracer, const color& c, bool depth_test);
void draw_bvh(Image& img, struct bvh_node* root, const ray_tracer& tracer,
              size_t level, bool depth_test);
