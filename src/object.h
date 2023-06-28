#pragma once

#include "aabb.h"
#include "material.h"

struct hittable {
  virtual ~hittable() = default;
  hittable() = default;

  explicit hittable(std::shared_ptr<material> m) : mat(std::move(m)) {}

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const = 0;

  virtual bool bounding_box(real_t time0, real_t time1, aabb& out) const = 0;

  virtual class bvh_node* as_bvh_node() { return nullptr; }

  std::shared_ptr<material> mat = nullptr;
};

struct sphere : public hittable {
  sphere(point3 cen, real_t r, std::shared_ptr<material> mat)
      : hittable(mat), center(cen), radius(r) {}

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const override;

  virtual bool bounding_box(real_t time0, real_t time1,
                            aabb& out) const override;

  point3 center;
  real_t radius;
};

// TODO: Remove this
struct moving_sphere : public hittable {
  moving_sphere(point3 cen0, point3 cen1, real_t t0, real_t t1, real_t r,
                std::shared_ptr<material> mat)
      : hittable(mat),
        center0(cen0),
        center1(cen1),
        time0(t0),
        time1(t1),
        radius(r) {}

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const override;

  point3 center(real_t time) const {
    return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
  }

  virtual bool bounding_box(real_t time0, real_t time1,
                            aabb& out) const override;

  point3 center0, center1;
  real_t time0, time1;
  real_t radius;
};

struct plane : public hittable {
  plane(point3 cen, vec3 n, std::shared_ptr<material> mat)
      : hittable(mat), center(cen), normal(n) {}

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const override;

  virtual bool bounding_box(real_t time0, real_t time1,
                            aabb& out) const override;

  point3 center;
  vec3 normal;
};

struct xy_rect : public hittable {
  xy_rect(real_t x0, real_t x1, real_t y0, real_t y1, real_t k,
          std::shared_ptr<material> mat)
      : hittable(mat), x0(x0), x1(x1), y0(y0), y1(y1), k(k) {}

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const override;

  virtual bool bounding_box(real_t time0, real_t time1,
                            aabb& out) const override;

  real_t x0, x1, y0, y1, k;
};