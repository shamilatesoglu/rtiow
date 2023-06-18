#pragma once

#include "common.h"

#include "material.h"
#include "vec.h"

struct ray {
  ray() {}

  ray(const point3& origin, const vec3& direction, real_t time = 0.0)
      : orig(origin), dir(direction), t(time) {}

  point3 origin() const { return orig; }

  vec3 direction() const { return dir; }

  point3 at(real_t t) const { return orig + t * dir; }

  const real_t& time() const { return t; }

  point3 orig;
  vec3 dir;

 protected:
  real_t t;
};

struct hit_record {
  point3 p;
  vec3 normal;
  real_t t;
  bool front_face;
  std::shared_ptr<material> mat = nullptr;

  inline void set_face_normal(const ray& r, const vec3& outward_normal) {
    front_face = r.direction().dot(outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};