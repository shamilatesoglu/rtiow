#pragma once

#include "object.h"

struct bvh_node : hittable {
  bvh_node(const std::vector<std::shared_ptr<hittable>>& src_objects,
           size_t start, size_t end, double time0, double time1);

  virtual bool hit(const ray& r, double t_min, double t_max,
                   hit_record& rec) const override;

  virtual bool bounding_box(double time0, double time1,
                            aabb& output_box) const override;

  std::shared_ptr<hittable> left;
  std::shared_ptr<hittable> right;

 protected:
  aabb box;
};