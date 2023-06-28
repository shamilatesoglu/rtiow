#include "draw.h"

#include "aabb.h"
#include "bvh.h"
#include "camera.h"
#include "ray_tracer.h"

void draw_line(Image& dst, point3 start, point3 end, color color,
               const ray_tracer& tracer) {
  auto start_ss =
    tracer.camera.screen_space(start, vec2(dst.width, dst.height));
  auto end_ss = tracer.camera.screen_space(end, vec2(dst.width, dst.height));
  if (!start_ss || !end_ss) {
    return;
  }

  auto x0 = start_ss->x();
  auto y0 = start_ss->y();
  auto x1 = end_ss->x();
  auto y1 = end_ss->y();

  ImageDrawLine(&dst, x0, y0, x1, y1, raylib_color(color));
}

void draw_aabb(Image& img, const aabb& box, const ray_tracer& tracer,
               const color& c) {
  auto p0 = box.min;
  auto p1 = point3(box.max.x(), box.min.y(), box.min.z());
  auto p2 = point3(box.max.x(), box.max.y(), box.min.z());
  auto p3 = point3(box.min.x(), box.max.y(), box.min.z());
  auto p4 = point3(box.min.x(), box.min.y(), box.max.z());
  auto p5 = point3(box.max.x(), box.min.y(), box.max.z());
  auto p6 = box.max;
  auto p7 = point3(box.min.x(), box.max.y(), box.max.z());

  std::vector<point3> points = {p0, p1, p2, p3, p4, p5, p6, p7};
  // Draw box
  draw_line(img, points[0], points[1], c, tracer);
  draw_line(img, points[1], points[2], c, tracer);
  draw_line(img, points[2], points[3], c, tracer);
  draw_line(img, points[3], points[0], c, tracer);
  draw_line(img, points[4], points[5], c, tracer);
  draw_line(img, points[5], points[6], c, tracer);
  draw_line(img, points[6], points[7], c, tracer);
  draw_line(img, points[7], points[4], c, tracer);
  draw_line(img, points[0], points[4], c, tracer);
  draw_line(img, points[1], points[5], c, tracer);
  draw_line(img, points[2], points[6], c, tracer);
  draw_line(img, points[3], points[7], c, tracer);
}

void draw_bvh(Image& img, bvh_node* root, const ray_tracer& tracer,
              size_t level) {
  auto bvh_box_color = color(0, 1, 0) * (level + 1) / 10;
  color bvh_leaf_color(1, 0, 0);
  while (root) {
    if (root->is_leaf()) {
      draw_aabb(img, root->box, tracer, bvh_leaf_color);
    } else {
      draw_aabb(img, root->box, tracer, bvh_box_color);
      draw_bvh(img, root->left->as_bvh_node(), tracer, level + 1);
      draw_bvh(img, root->right->as_bvh_node(), tracer, level + 1);
    }
    break;
  }
}
