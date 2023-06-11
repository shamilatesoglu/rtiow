#include <iostream>

#include "raylib.h"

#include "object.h"
#include "ray.h"
#include "vec.h"

#include "stopwatch.h"

color background_color(const ray& r) {
  vec3 unit_direction = unit_vector(r.direction());
  auto t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

color real_to_screen(const color& c) {
  return 255.999 * c;
}

// c: Color in [0, 1]
void draw_pixel(int x, int y, const color& c) {
  auto sc = real_to_screen(c);
  auto ir = static_cast<uint8_t>(sc.x());
  auto ig = static_cast<uint8_t>(sc.y());
  auto ib = static_cast<uint8_t>(sc.z());
  DrawPixel(x, y, Color{ir, ig, ib, 255});
}

int main(void) {
  // Image
  const auto aspect_ratio = 16.0 / 9.0;
  const int image_width = 800;
  const int image_height = static_cast<int>(image_width / aspect_ratio);

  // Camera
  auto viewport_height = 2.0;
  auto viewport_width = aspect_ratio * viewport_height;
  auto focal_length = 1.0;

  auto origin = point3(0, 0, 0);
  auto horizontal = vec3(viewport_width, 0, 0);
  auto vertical = vec3(0, viewport_height, 0);
  auto lower_left_corner =
      origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);

  InitWindow(image_width, image_height, "RTIOW");

  std::vector<std::shared_ptr<hittable>> objects;
  objects.push_back(std::make_shared<sphere>(point3(0, 0, -1), 0.5));

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    stopwatch sw;
    for (int j = 0; j < image_height; ++j) {
      for (int i = 0; i < image_width; ++i) {
        auto u = static_cast<REAL_T>(i) / (image_width - 1);
        auto v = static_cast<REAL_T>(j) / (image_height - 1);
        // Flip the image vertically
        v = 1.0 - v;
        ray r(origin, lower_left_corner + u * horizontal + v * vertical);
        bool hit = false;
        for (auto& obj : objects) {
          hit_record rec;
          if (obj->hit(r, 0, INFINITY, rec)) {
            draw_pixel(i, j, 0.5 * (rec.normal + color(1, 1, 1)));
            hit = true;
            continue;
          }
        }
        if (!hit) {
          draw_pixel(i, j, background_color(r));
        }
      }
    }
    EndDrawing();
    std::cout << "\rElapsed time: " << sw.elapsed_str() << std::flush;
  }

  CloseWindow();
  return 0;
}