#pragma once

#include "common.h"

struct material {
  virtual bool scatter(const class ray& r_in,
                       const class hit_record& rec,
                       color& attenuation,
                       ray& scattered) const = 0;
};

class lambertian : public material {
    public:
        lambertian(const color& a) : albedo(a) {}

        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override;

    public:
        color albedo;
};