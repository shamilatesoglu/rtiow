#include "bvh.h"

#include <iostream>

std::shared_ptr<bvh_node> bvh_node::build(
  std::vector<std::shared_ptr<hittable>>& objects, real_t time0, real_t time1) {
  // Build the tree by merging the bounding volumes of the two nearest
  // bounding volumes, until there is only the root node left. If the
  // object's bounding box can not be computed, then it is not included in the tree.
  bool stop = false;
  while (!stop) {
    real_t min_distance = std::numeric_limits<real_t>::max();
    int min_i = -1;
    int min_j = -1;
    for (size_t i = 0; i < objects.size(); ++i) {
      aabb box_i;
      if (!objects[i]->bounding_box(time0, time1, box_i))
        continue;
      for (size_t j = i + 1; j < objects.size(); ++j) {
        aabb box_j;
        if (!objects[j]->bounding_box(time0, time1, box_j))
          continue;
        real_t dst = box_i.distance_sq(box_j);
        if (dst < min_distance) {
          min_distance = dst;
          min_i = i;
          min_j = j;
        }
      }
    }
    if (min_i == -1 || min_j == -1) {
      stop = true;
      continue;
    }
    aabb box_i;
    objects[min_i]->bounding_box(time0, time1, box_i);
    aabb box_j;
    objects[min_j]->bounding_box(time0, time1, box_j);
    objects.push_back(std::make_shared<bvh_node>(objects[min_i], objects[min_j],
                                                 box_i.surrounding(box_j)));
    objects.erase(objects.begin() + min_j);
    objects.erase(objects.begin() + min_i);
  }

  // If there is only one object left, then it is the root node.
  std::shared_ptr<hittable> left, right;
  aabb box;
  if (objects.size() == 1) {
    left = objects[0];
  } else if (objects.size() == 2) {
    left = objects[0];
    right = objects[1];
    left->bounding_box(time0, time1, box);
    aabb right_box;
    right->bounding_box(time0, time1, right_box);
    box = box.surrounding(right_box);
  } else {
    std::cerr << "bvh_node::bvh_node: objects.size() = " << objects.size()
              << std::endl;
  }
  return std::make_shared<bvh_node>(left, right, box);
}

bool bvh_node::bounding_box(real_t time0, real_t time1,
                            aabb& output_box) const {
  output_box = box;
  return true;
}

bool bvh_node::hit(const ray& r, real_t t_min, real_t t_max,
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
