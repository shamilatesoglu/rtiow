#pragma once

#include "aabb.h"
#include "material.h"
#include "stopwatch.h"

// stl
#include <iostream>
#include <shared_mutex>

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

struct hittable_list : public hittable {

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const override {
    hit_record cur_rec;
    bool hit_anything = false;
    auto closest_so_far = t_max;
    for (const auto& obj : objects) {
      if (obj->hit(r, t_min, closest_so_far, cur_rec)) {
        hit_anything = true;
        closest_so_far = cur_rec.t;
        rec = cur_rec;
      }
    }
    return hit_anything;
  }

  virtual bool bounding_box(double time0, double time1,
                            aabb& output_box) const override;

  void add_object(std::shared_ptr<hittable> obj) {
    objects.emplace_back(std::move(obj));
  }

  void clear_objects() {
    objects.clear();
    bvh_root.reset();
  }

  void build_bvh();

  std::shared_ptr<struct bvh_node> bvh_root;
  std::vector<std::shared_ptr<hittable>> objects;
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

struct xz_rect : public hittable {
  xz_rect(real_t x0, real_t x1, real_t z0, real_t z1, real_t k,
          std::shared_ptr<material> mat)
      : hittable(mat), x0(x0), x1(x1), z0(z0), z1(z1), k(k) {}

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const override;

  virtual bool bounding_box(real_t time0, real_t time1,
                            aabb& out) const override;

  real_t x0, x1, z0, z1, k;
};

struct yz_rect : public hittable {
  yz_rect(real_t y0, real_t y1, real_t z0, real_t z1, real_t k,
          std::shared_ptr<material> mat)
      : hittable(mat), y0(y0), y1(y1), z0(z0), z1(z1), k(k) {}

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const override;

  virtual bool bounding_box(real_t time0, real_t time1,
                            aabb& out) const override;

  real_t y0, y1, z0, z1, k;
};

struct box : public hittable {
  box(point3 p0, point3 p1, std::shared_ptr<material> mat)
      : hittable(mat), p0(p0), p1(p1) {
    sides.add_object(
      std::make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), mat));
    sides.add_object(
      std::make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), mat));
    sides.add_object(
      std::make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), mat));
    sides.add_object(
      std::make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), mat));
    sides.add_object(
      std::make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), mat));
    sides.add_object(
      std::make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), mat));
    sides.add_object(
      std::make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), mat));
    sides.add_object(
      std::make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), mat));
  }

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const override {
    return sides.hit(r, t_min, t_max, rec);
  }

  virtual bool bounding_box(real_t time0, real_t time1,
                            aabb& out) const override {
    out = aabb(p0, p1);
    return true;
  }

  point3 p0, p1;
  hittable_list sides;
};

struct translate : public hittable {
  translate(std::shared_ptr<hittable> p, const vec3& displacement)
      : hittable(p->mat), ptr(p), offset(displacement) {}

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const override;

  virtual bool bounding_box(real_t time0, real_t time1,
                            aabb& out) const override;

  virtual class bvh_node* as_bvh_node() override { return ptr->as_bvh_node(); }

  std::shared_ptr<hittable> ptr;
  vec3 offset;
};

struct rotate_y : public hittable {
  rotate_y(std::shared_ptr<hittable> p, real_t angle);

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const override;

  virtual bool bounding_box(real_t time0, real_t time1,
                            aabb& out) const override;

  virtual class bvh_node* as_bvh_node() override { return ptr->as_bvh_node(); }

  std::shared_ptr<hittable> ptr;
  real_t sin_theta;
  real_t cos_theta;
  bool has_box;
  aabb bbox;
};