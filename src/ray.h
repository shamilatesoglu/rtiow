#pragma once

#include "common.h"

#include "material.h"
#include "vec.h"

struct ray {
  ray() {}
  ray(const point3& origin, const vec3& direction)
      : orig(origin), dir(direction) {}

  point3 origin() const { return orig; }
  vec3 direction() const { return dir; }

  point3 at(REAL_T t) const { return orig + t * dir; }

  point3 orig;
  vec3 dir;
};

struct hit_record {
  point3 p;
  vec3 normal;
  REAL_T t;
  bool front_face;
  std::shared_ptr<material> mat = nullptr;
  inline void set_face_normal(const ray& r, const vec3& outward_normal) {
    front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

struct hittable {
  virtual ~hittable() = default;
  explicit hittable(std::shared_ptr<material> m) : mat(std::move(m)) {}
  virtual bool hit(const ray& r,
                   REAL_T t_min,
                   REAL_T t_max,
                   hit_record& rec) const = 0;
  std::shared_ptr<material> mat = nullptr;
};