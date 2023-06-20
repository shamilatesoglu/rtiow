#pragma once

#include "ray.h"
#include "vec.h"

struct aabb {
  aabb() {}

  aabb(const point3& a, const point3& b) : min(a), max(b) {}

  bool overlaps(const aabb& other) const {
    return (min.x() <= other.max.x() && max.x() >= other.min.x()) &&
           (min.y() <= other.max.y() && max.y() >= other.min.y()) &&
           (min.z() <= other.max.z() && max.z() >= other.min.z());
  }

  bool hit(const ray& r, real_t t_min, real_t t_max) const {
    for (int a = 0; a < 3; a++) {
      real_t inv_d = 1.0f / r.direction()[a];
      real_t t0 = (min[a] - r.origin()[a]) * inv_d;
      real_t t1 = (max[a] - r.origin()[a]) * inv_d;
      if (inv_d < 0.0f)
        std::swap(t0, t1);
      t_min = t0 > t_min ? t0 : t_min;
      t_max = t1 < t_max ? t1 : t_max;
      if (t_max <= t_min)
        return false;
    }
    return true;
  }

  real_t volume() const {
    auto d = max - min;
    return d.x() * d.y() * d.z();
  }

  point3 min;
  point3 max;
};

inline aabb surrounding_box(aabb box0, aabb box1) {
  point3 small(fmin(box0.min.x(), box1.min.x()),
               fmin(box0.min.y(), box1.min.y()),
               fmin(box0.min.z(), box1.min.z()));

  point3 big(fmax(box0.max.x(), box1.max.x()), fmax(box0.max.y(), box1.max.y()),
             fmax(box0.max.z(), box1.max.z()));

  return aabb(small, big);
}