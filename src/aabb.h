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

  aabb intersect(const aabb& other) const {
    point3 small(fmax(min.x(), other.min.x()), fmax(min.y(), other.min.y()),
                 fmax(min.z(), other.min.z()));

    point3 big(fmin(max.x(), other.max.x()), fmin(max.y(), other.max.y()),
               fmin(max.z(), other.max.z()));

    return aabb(small, big);
  }

  real_t distance_sq(const aabb& other) const {
    aabb intersection = intersect(other);
    real_t d_sq = 0.0;
    for (int i = 0; i < 3; i++) {
        if (intersection.min[i] > intersection.max[i]) {
            d_sq += (intersection.min[i] - intersection.max[i]) * (intersection.min[i] - intersection.max[i]);
        }
    }
    return d_sq;
  }

  aabb surrounding(const aabb& other) const {
    point3 small(fmin(min.x(), other.min.x()), fmin(min.y(), other.min.y()),
                 fmin(min.z(), other.min.z()));

    point3 big(fmax(max.x(), other.max.x()), fmax(max.y(), other.max.y()),
               fmax(max.z(), other.max.z()));

    return aabb(small, big);
  }

  point3 min;
  point3 max;
};