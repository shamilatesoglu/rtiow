#include "draw.h"

#include "aabb.h"
#include "camera.h"
#include "bvh.h"

void draw_line(Image& img, const point2& p0, const point2& p1, const color& c) {
    // Draw line to image
    Color color = { static_cast<uint8_t>(c.x() * 255.999),
        static_cast<uint8_t>(c.y() * 255.999),
        static_cast<uint8_t>(c.z() * 255.999), 255 };
    ImageDrawLine(&img, p0.x(), p0.y(), p1.x(), p1.y(), color);
}

void draw_aabb(Image& img, const aabb& box, const camera& cam, const color& c) {
    auto p0 = box.min;
    auto p1 = point3(box.max.x(), box.min.y(), box.min.z());
    auto p2 = point3(box.max.x(), box.max.y(), box.min.z());
    auto p3 = point3(box.min.x(), box.max.y(), box.min.z());
    auto p4 = point3(box.min.x(), box.min.y(), box.max.z());
    auto p5 = point3(box.max.x(), box.min.y(), box.max.z());
    auto p6 = box.max;
    auto p7 = point3(box.min.x(), box.max.y(), box.max.z());

    std::vector<point3> points = { p0, p1, p2, p3, p4, p5, p6, p7 };
    std::vector<point2> points2d(points.size());
    bool any_behind = false;
    for (size_t i = 0; i < points.size(); ++i) {
        auto p = cam.screen_space(points[i], vec2(img.width, img.height));
        if (!p) {
            any_behind = true;
            break;
        }
        points2d[i] = *p;
    }
    if (any_behind) {
        return;
    }
    // Draw box
    draw_line(img, points2d[0], points2d[1], c);
    draw_line(img, points2d[1], points2d[2], c);
    draw_line(img, points2d[2], points2d[3], c);
    draw_line(img, points2d[3], points2d[0], c);
    draw_line(img, points2d[4], points2d[5], c);
    draw_line(img, points2d[5], points2d[6], c);
    draw_line(img, points2d[6], points2d[7], c);
    draw_line(img, points2d[7], points2d[4], c);
    draw_line(img, points2d[0], points2d[4], c);
    draw_line(img, points2d[1], points2d[5], c);
    draw_line(img, points2d[2], points2d[6], c);
    draw_line(img, points2d[3], points2d[7], c);
}

void draw_bvh(Image& img, bvh_node* root, const camera& cam, size_t depth) {
    auto bvh_box_color = color(0, 1, 0) * (depth + 1) / 10;
    color bvh_leaf_color(1, 0, 0);
    while (root) {
        if (root->is_leaf()) {
            draw_aabb(img, root->box, cam, bvh_leaf_color);
        }
        else {
            draw_aabb(img, root->box, cam, bvh_box_color);
            draw_bvh(img, root->left->as_bvh_node(), cam, depth + 1);
            draw_bvh(img, root->right->as_bvh_node(), cam, depth + 1);
        }
        break;
    }
}
