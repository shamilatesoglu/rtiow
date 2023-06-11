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
  ray_tracer(const class camera &cam, uint8_t multi_sample_count,
             size_t max_depth, size_t image_width, size_t image_height)
      : multi_sample_count(multi_sample_count), max_depth(max_depth),
        camera(cam), image_width(image_width), image_height(image_height) {
    pixel_width = 1.0 / image_width;
    pixel_height = 1.0 / image_height;
  }

  void add_object(std::shared_ptr<hittable> obj) {
    objects.emplace_back(std::move(obj));
  }
  void clear_objects() { objects.clear(); }

  color compute(REAL_T u, REAL_T v) {
    color c;
    for (int i = 0; i < multi_sample_count; ++i) {
      REAL_T up = u + random_real() * pixel_width;
      REAL_T vp = v + random_real() * pixel_height;
      color sample_color;
      ray r = camera.get_ray(u, v);
      fire_ray(r, sample_color, max_depth);
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
  void fire_ray(const ray &r, color &c, size_t depth) {
    if (depth <= 0) {
      c = color(0, 0, 0);
      return;
    }
    hit_record rec;
    if (hit(r, 0.001, INFINITY, rec)) {
      ray scattered;
      color attenuation;
      if (rec.mat->scatter(r, rec, attenuation, scattered)) {
        fire_ray(scattered, c, depth - 1);
        c = c * attenuation;
      } else {
        c = color(0, 0, 0);
      }
    } else {
      c = background_color(r);
    }
  }

  bool hit(const ray &r, REAL_T t_min, REAL_T t_max, hit_record &rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = t_max;
    for (const auto &obj : objects) {
      if (obj->hit(r, t_min, closest_so_far, temp_rec)) {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec;
      }
    }
    return hit_anything;
  }

  uint8_t multi_sample_count;
  size_t max_depth;
  const camera &camera;
  size_t image_width;
  size_t image_height;
  REAL_T pixel_width;
  REAL_T pixel_height;
  std::vector<std::shared_ptr<hittable>> objects;
};

int main(void) {
  // Image
  const auto aspect_ratio = 16.0 / 9.0;
  const size_t image_width = 800;
  const size_t image_height = static_cast<size_t>(image_width / aspect_ratio);

  InitWindow(image_width, image_height, "RTIOW");

  auto diffuse_mat = std::make_shared<lambertian>(color(0.9, 0.5, 0.5));
  auto plane_mat = std::make_shared<lambertian>(color(0.5, 0.5, 0.5));

  camera cam(aspect_ratio);
  ray_tracer tracer(cam, 100, 50, image_width, image_height);
  tracer.add_object(
      std::make_shared<plane>(point3(0, -0.5, 0), vec3(0, 1, 0), plane_mat));
  tracer.add_object(
      std::make_shared<sphere>(point3(0, 0, -1), 0.5, diffuse_mat));
  tracer.add_object(
      std::make_shared<sphere>(point3(1.2, 0, -1), 0.2, diffuse_mat));
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
          auto c = tracer.compute(u, v);
          color_map[j] = c;
        }
      });
    }
    pool.wait();
    for (int j = 0; j < image_height; ++j) {
      for (int i = 0; i < image_width; ++i) {
        auto const &c = color_map[j * image_width + i];
        draw_pixel(i, j, c);
      }
    }
    EndDrawing();
    std::cout << "\rElapsed time: " << sw.elapsed_str() << std::flush;
  }

  CloseWindow();
  return 0;
}
