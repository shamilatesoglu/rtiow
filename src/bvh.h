#pragma once

#include "object.h"

struct bvh_node : hittable {

  // Modifies the objects vector with the new tree. If the object's bounding box
  // can not be computed, then it is not included in the tree, but it'll still
  // be available in the objects vector.
  static std::shared_ptr<bvh_node> build(
    std::vector<std::shared_ptr<hittable>>& objects, real_t time0,
    real_t time1);

  bvh_node(std::shared_ptr<hittable> left, std::shared_ptr<hittable> right,
           const aabb& box)
      : left(left), right(right), box(box) {}

  virtual bool hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const override;

  virtual bool bounding_box(real_t time0, real_t time1,
                            aabb& output_box) const override;

  bvh_node* as_bvh_node() override { return this; }

  bool is_leaf()
    const {  // If any is null, or none of them is bvh_node, then it's a leaf.
    if (!left || !right)
      return true;
    return !left->as_bvh_node() && !right->as_bvh_node();
  }

  std::shared_ptr<hittable> left;
  std::shared_ptr<hittable> right;
  aabb box;
};
