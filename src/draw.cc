#include "draw.h"

#include "aabb.h"
#include "bvh.h"
#include "camera.h"
#include "ray_tracer.h"

// Taken in part from raylib's rtextures.c
void draw_line_with_depth_test(Image& dst, point3 start, point3 end,
                               color color, const ray_tracer& tracer,
                               bool depth_test) {
  // Using Bresenham's algorithm as described in
  // Drawing Lines with Pixels - Joshua Scott - March 2012
  // https://classic.csunplugged.org/wp-content/uploads/2014/12/Lines.pdf
  auto start_ss =
    tracer.camera.screen_space(start, vec2(dst.width, dst.height));
  auto end_ss = tracer.camera.screen_space(end, vec2(dst.width, dst.height));
  if (!start_ss || !end_ss) {
    return;
  }

  auto depth_start = tracer.camera.depth_to(start);
  auto depth_end = tracer.camera.depth_to(end);

  int start_pos_x = static_cast<int>(start_ss->x());
  int start_pos_y = static_cast<int>(start_ss->y());
  int end_pos_x = static_cast<int>(end_ss->x());
  int end_pos_y = static_cast<int>(end_ss->y());

  std::vector<std::pair<int, int>> raster;

  int change_in_x = (end_pos_x - start_pos_x);
  int abs_change_in_x = (change_in_x < 0) ? -change_in_x : change_in_x;
  int change_in_y = (end_pos_y - start_pos_y);
  int abs_change_in_y = (change_in_y < 0) ? -change_in_y : change_in_y;

  int start_u, start_v, end_u,
    step_v;  // Substitutions, either U = X, V = Y or vice versa. See loop at end of function
  int A, B, P;  // See linked paper above, explained down in the main loop
  int reversed_xy = (abs_change_in_y < abs_change_in_x);

  if (reversed_xy) {
    A = 2 * abs_change_in_y;
    B = A - 2 * abs_change_in_x;
    P = A - abs_change_in_x;

    if (change_in_x > 0) {
      start_u = start_pos_x;
      start_v = start_pos_y;
      end_u = end_pos_x;
    } else {
      start_u = end_pos_x;
      start_v = end_pos_y;
      end_u = start_pos_x;

      // Since start and end are reversed
      change_in_x = -change_in_x;
      change_in_y = -change_in_y;
    }

    step_v = (change_in_y < 0) ? -1 : 1;

    raster.emplace_back(std::pair<int, int>(start_u, start_v));
  } else {
    A = 2 * abs_change_in_x;
    B = A - 2 * abs_change_in_y;
    P = A - abs_change_in_y;

    if (change_in_y > 0) {
      start_u = start_pos_y;
      start_v = start_pos_x;
      end_u = end_pos_y;
    } else {
      start_u = end_pos_y;
      start_v = end_pos_x;
      end_u = start_pos_y;

      // Since start and end are reversed
      change_in_x = -change_in_x;
      change_in_y = -change_in_y;
    }

    step_v = (change_in_x < 0) ? -1 : 1;
    raster.emplace_back(std::pair<int, int>(
      start_v,
      start_u));  // ... but need to be reversed here. Repeated in the main loop below
  }

  // We already drew the start point. If we started at startU + 0, the line would be crooked and too short
  for (int u = start_u + 1, v = start_v; u <= end_u; u++) {
    if (P >= 0) {
      v +=
        step_v;  // Adjusts whenever we stray too far from the direct line. Details in the linked paper above
      P += B;    // Remembers that we corrected our path
    } else
      P += A;  // Remembers how far we are from the direct line

    if (reversed_xy)
      raster.emplace_back(std::pair<int, int>(u, v));
    else
      raster.emplace_back(std::pair<int, int>(v, u));
  }

  for (int i = 0; i < raster.size(); ++i) {
    auto& point = raster[i];
    if (depth_test) {
      auto d = lerp(static_cast<float>(i) / raster.size(), depth_start, depth_end);
      auto cur_depth = tracer.depth_at(point.first, point.second);
      if (d < cur_depth) {
        write_pixel(dst, point.first, point.second, color);
      }
    } else {
      write_pixel(dst, point.first, point.second, color);
    }
  }
}

void draw_aabb(Image& img, const aabb& box, const ray_tracer& tracer,
               const color& c, bool depth_test) {
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
  draw_line_with_depth_test(img, points[0], points[1], c, tracer, depth_test);
  draw_line_with_depth_test(img, points[1], points[2], c, tracer, depth_test);
  draw_line_with_depth_test(img, points[2], points[3], c, tracer, depth_test);
  draw_line_with_depth_test(img, points[3], points[0], c, tracer, depth_test);
  draw_line_with_depth_test(img, points[4], points[5], c, tracer, depth_test);
  draw_line_with_depth_test(img, points[5], points[6], c, tracer, depth_test);
  draw_line_with_depth_test(img, points[6], points[7], c, tracer, depth_test);
  draw_line_with_depth_test(img, points[7], points[4], c, tracer, depth_test);
  draw_line_with_depth_test(img, points[0], points[4], c, tracer, depth_test);
  draw_line_with_depth_test(img, points[1], points[5], c, tracer, depth_test);
  draw_line_with_depth_test(img, points[2], points[6], c, tracer, depth_test);
  draw_line_with_depth_test(img, points[3], points[7], c, tracer, depth_test);
}

void draw_bvh(Image& img, bvh_node* root, const ray_tracer& tracer,
              size_t level, bool depth_test) {
  auto bvh_box_color = color(0, 1, 0) * (level + 1) / 10;
  color bvh_leaf_color(1, 0, 0);
  while (root) {
    if (root->is_leaf()) {
      draw_aabb(img, root->box, tracer, bvh_leaf_color, depth_test);
    } else {
      draw_aabb(img, root->box, tracer, bvh_box_color, depth_test);
      draw_bvh(img, root->left->as_bvh_node(), tracer, level + 1, depth_test);
      draw_bvh(img, root->right->as_bvh_node(), tracer, level + 1, depth_test);
    }
    break;
  }
}
