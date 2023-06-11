#pragma once

#include "material.h"
#include "ray.h"

struct sphere : public hittable {
  sphere(point3 cen, REAL_T r, std::shared_ptr<material> mat)
      : hittable(mat), center(cen), radius(r) {}

  virtual bool hit(const ray& r,
                   REAL_T t_min,
                   REAL_T t_max,
                   hit_record& rec) const override;

  point3 center;
  REAL_T radius;
};