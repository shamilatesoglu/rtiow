#pragma once

#include "vec.h"
#include "texture.h"

struct material {
  virtual bool scatter(const class ray& r_in,
                       const class hit_record& rec,
                       color& attenuation,
                       ray& scattered) const = 0;
};

struct lambertian : public material {
  lambertian(std::shared_ptr<texture> a) : albedo(std::move(a)) {}
  lambertian(const color& a) : albedo(std::make_shared<solid_color>(a)) {}

  virtual bool scatter(const ray& r_in,
                       const hit_record& rec,
                       color& attenuation,
                       ray& scattered) const override;

  std::shared_ptr<texture> albedo;
};

struct metal : public material {
  metal(const color& a, real_t f) : albedo(a), fuzz(f < 1 ? f : 1) {}

  virtual bool scatter(const ray& r_in,
                       const hit_record& rec,
                       color& attenuation,
                       ray& scattered) const override;

  color albedo;
  real_t fuzz;
};

struct glass : public material {
  glass(const color& a, real_t ior) : albedo(a), ior(ior) {}

  virtual bool scatter(const ray& r_in,
                       const hit_record& rec,
                       color& attenuation,
                       ray& scattered) const override;

  color albedo;
  real_t ior;
};