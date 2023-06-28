#pragma once

#include "vec.h"

struct texture {
  virtual color value(real_t u, real_t v, const point3& p) const = 0;
};

struct solid_color : public texture {
  solid_color() {}

  solid_color(color c) : color_value(c) {}

  solid_color(real_t red, real_t green, real_t blue)
      : solid_color(color(red, green, blue)) {}

  virtual color value(real_t u, real_t v, const vec3& p) const override {
    return color_value;
  }

  color color_value;
};

class plane_checker_texture : public texture {
 public:
  plane_checker_texture(std::shared_ptr<texture> even,
                        std::shared_ptr<texture> odd)
      : even(std::move(even)), odd(std::move(odd)) {}

  plane_checker_texture(color c1, color c2)
      : even(std::make_shared<solid_color>(c1)),
        odd(std::make_shared<solid_color>(c2)) {}

  virtual color value(real_t u, real_t v, const point3& p) const override {
    auto sines = sin(10 * p.x()) * sin(10 * p.z());
    if (sines < 0) {
      return odd->value(u, v, p);
    } else {
      return even->value(u, v, p);
    }
  }

 protected:
  std::shared_ptr<texture> odd;
  std::shared_ptr<texture> even;
};