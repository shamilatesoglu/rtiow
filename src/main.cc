#include <iostream>

#include "raylib.h"

#include "camera.h"
#include "object.h"
#include "ray.h"
#include "thread_pool.h"
#include "vec.h"

#include "stopwatch.h"

color real_to_screen(const color &c) { return 255.999 * c; }

// c: Color in [0, 1]
void draw_pixel(int x, int y, const color &c) {
  auto sc = real_to_screen(c);
  auto ir = static_cast<uint8_t>(sc.x());
  auto ig = static_cast<uint8_t>(sc.y());
  auto ib = static_cast<uint8_t>(sc.z());
  DrawPixel(x, y, Color{ir, ig, ib, 255});
}

struct ray_tracer {
  ray_tracer(const class camera &cam, uint8_t multi_sample_count)
      : multi_sample_count(multi_sample_count), camera(cam) {}

  void add_object(std::shared_ptr<hittable> obj) {
    objects.emplace_back(std::move(obj));
  }
  void clear_objects() { objects.clear(); }

  color compute(REAL_T u, REAL_T v, size_t image_width, size_t image_height) {
    REAL_T pixel_width = 1.0 / image_width;
    REAL_T pixel_height = 1.0 / image_height;
    color c;
    for (int i = 0; i < multi_sample_count; ++i) {
      REAL_T up = u + random_real() * pixel_width;
      REAL_T vp = v + random_real() * pixel_height;
      color sample_color;
      hit(up, vp, sample_color);
      c += sample_color;
    }
    return c / multi_sample_count;
  }

  color background_color(const ray &r) {
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
  }

protected:
  bool hit(REAL_T u, REAL_T v, color &out_color) {
    ray r = camera.get_ray(u, v);
    bool hit_anything = false;
    hit_record rec;
    REAL_T closest_so_far = INFINITY;
    for (const auto &obj : objects) {
      if (obj->hit(r, 0, closest_so_far, rec)) {
        hit_anything = true;
        closest_so_far = rec.t;
        out_color = 0.5 * (rec.normal + color(1, 1, 1));
      }
    }
    if (!hit_anything) {
      out_color = background_color(r);
    }
    return hit_anything;
  }

  uint8_t multi_sample_count;
  const camera &camera;
  std::vector<std::shared_ptr<hittable>> objects;
};

int main(void) {
  // Image
  const auto aspect_ratio = 16.0 / 9.0;
  const size_t image_width = 800;
  const size_t image_height = static_cast<size_t>(image_width / aspect_ratio);

  InitWindow(image_width, image_height, "RTIOW");

  camera cam(aspect_ratio);
  ray_tracer tracer(cam, 1);
  tracer.add_object(std::make_shared<sphere>(point3(0, 0, -1), 0.5));
  tracer.add_object(std::make_shared<sphere>(point3(1.2, 0, -1), 0.2));
  thread_pool pool(8);

  std::vector<color> color_map(image_height * image_width);
  for (size_t i = 0; i < image_height * image_width; ++i) {
    color_map[i] = color(0, 0, 0);
  }

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    stopwatch sw;
    size_t segments = pool.pool_size();
    for (size_t i = 0; i < segments; ++i) {
      pool.enqueue([&, i]() {
        for (size_t j = i; j < image_height * image_width; j += segments) {
          size_t x = j % image_width;
          size_t y = j / image_width;
          REAL_T u = static_cast<REAL_T>(x) / (image_width - 1);
          REAL_T v = static_cast<REAL_T>(y) / (image_height - 1);
          v = 1.0 - v;
          auto c = tracer.compute(u, v, image_width, image_height);
          color_map[j] = c;
        }
      });
    }
    pool.wait();
    for (int j = 0; j < image_height; ++j) {
      for (int i = 0; i < image_width; ++i) {
        auto const & c = color_map[j * image_width + i];
        draw_pixel(i, j, c);
      }
    }
    EndDrawing();
    std::cout << "\rElapsed time: " << sw.elapsed_str() << std::flush;
  }

  CloseWindow();
  return 0;
}