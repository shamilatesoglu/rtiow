#include "bvh.h"

#include <iostream>

bvh_node::bvh_node(const std::vector<std::shared_ptr<hittable>>& src_objects,
                   double time0, double time1) {
  // Construct a BVH tree, according to the following constraints:
  // 1. Lower down the tree, the bounding volumes should be smaller.
  // 2. The bounding volumes should overlap as little as possible.
  // 3. The bounding volumes should be as small as possible.
  // 4. The tree should be as balanced as possible.

  auto objects = src_objects;
  // Build the first level with all the objects, containing only leaf nodes.
  for (size_t i = 0; i < objects.size(); ++i) {
    aabb box;
    objects[i]->bounding_box(time0, time1, box);
    objects[i] = std::make_shared<bvh_node>(objects[i], objects[i], box, true);
  }

  // Build the rest of the tree by merging the bounding volumes of the two nearest
  // bounding volumes, until there is only the root node left.
  while (objects.size() > 1) {
    real_t min_distance = std::numeric_limits<real_t>::max();
    int min_i = -1;
    int min_j = -1;
    for (size_t i = 0; i < objects.size(); ++i) {
      for (size_t j = i + 1; j < objects.size(); ++j) {
        aabb box_i;
        aabb box_j;
        objects[i]->bounding_box(time0, time1, box_i);
        objects[j]->bounding_box(time0, time1, box_j);
        real_t dst = box_i.distance_sq(box_j);
        if (dst < min_distance) {
          min_distance = dst;
          min_i = i;
          min_j = j;
        }
      }
    }

    auto left = objects[min_i];
    auto right = objects[min_j];
    aabb box_left, box_right;
    left->bounding_box(time0, time1, box_left);
    right->bounding_box(time0, time1, box_right);
    objects[min_i] = std::make_shared<bvh_node>(
      left, right, box_left.surrounding(box_right), false);
    objects.erase(objects.begin() + min_j);
  }

  left = objects[0];
  right = objects[0];
  left->bounding_box(time0, time1, box);
  leaf = false;
}

bool bvh_node::bounding_box(double time0, double time1,
                            aabb& output_box) const {
  output_box = box;
  return true;
}

bool bvh_node::hit(const ray& r, double t_min, double t_max,
                   hit_record& rec) const {
  if (!box.hit(r, t_min, t_max))
    return false;

  bool hit_left = false;
  bool hit_right = false;
  if (left)
    hit_left = left->hit(r, t_min, t_max, rec);
  if (right)
    hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

  return hit_left || hit_right;
}