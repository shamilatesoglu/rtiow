#include <iostream>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "camera.h"
#include "object.h"
#include "ray.h"
#include "raylib.h"
#include "stopwatch.h"
#include "thread_pool.h"
#include "vec.h"

color real_to_screen(const color& c) {
  return 255.999 * c;
}

color read_pixel(const Image& image, int x, int y) {
  Color c = GetImageColor(image, x, y);
  return color(c.r, c.g, c.b) / 255.999;
}

void write_pixel(Image& image, int x, int y, const color& c) {
  auto sc = real_to_screen(c);
  auto ir = static_cast<uint8_t>(sc.x());
  auto ig = static_cast<uint8_t>(sc.y());
  auto ib = static_cast<uint8_t>(sc.z());
  Color color = {ir, ig, ib, 255};
  ImageDrawPixel(&image, x, y, color);
}

struct ray_tracer {
  ray_tracer(const class camera& cam, size_t sample_count, size_t max_depth,
             size_t image_width, size_t image_height)
      : sample_count(sample_count),
        max_depth(max_depth),
        camera(cam),
        image_width(image_width),
        image_height(image_height) {
    pixel_width = 1.0 / image_width;
    pixel_height = 1.0 / image_height;
  }

  void add_object(std::shared_ptr<hittable> obj) {
    objects.emplace_back(std::move(obj));
  }

  void clear_objects() { objects.clear(); }

  color compute(REAL_T u, REAL_T v) {
    color c;
    for (int i = 0; i < sample_count; ++i) {
      REAL_T up = u + random_real() * pixel_width;
      REAL_T vp = v + random_real() * pixel_height;
      color sample_color;
      ray r = camera.ray_to(up, vp);
      fire_ray(r, sample_color, max_depth);
      c += sample_color;
    }
    return c / sample_count;
  }

  color background_color(const ray& r) {
    vec3 unit_direction = r.direction().normalized();
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
  }

  size_t sample_count;
  size_t max_depth;

 protected:
  void fire_ray(const ray& r, color& c, size_t depth) {
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
        c *= attenuation;
      } else {
        c = color(0, 0, 0);
      }
    } else {
      c = background_color(r);
    }
  }

  bool hit(const ray& r, REAL_T t_min, REAL_T t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = t_max;
    for (const auto& obj : objects) {
      if (obj->hit(r, t_min, closest_so_far, temp_rec)) {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec;
      }
    }
    return hit_anything;
  }

  const camera& camera;
  size_t image_width;
  size_t image_height;
  REAL_T pixel_width;
  REAL_T pixel_height;
  std::vector<std::shared_ptr<hittable>> objects;
};

void scatter_objects(ray_tracer& tracer) {
  std::vector<std::shared_ptr<sphere>> spheres;
  // Place 3 big spheres
  auto material1 = std::make_shared<glass>(color(1, 1, 1), 1.5);
  spheres.emplace_back(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

  auto material2 = std::make_shared<lambertian>(color(0.4, 0.2, 0.1));
  spheres.emplace_back(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

  auto material3 = std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
  spheres.emplace_back(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

  for (size_t i = 0; i < 100;) {
    auto choose_mat = random_real();
    auto radius = random_real(0.05, 0.25);
    auto center = vec3(random_real(-10, 10), radius, random_real(-10, 10));

    if ((center - point3(4, 0.2, 0)).length() > 0.9) {
      std::shared_ptr<material> sphere_material;

      if (choose_mat < 0.8) {
        // diffuse
        auto albedo = color::random() * color::random();
        sphere_material = std::make_shared<lambertian>(albedo);

      } else if (choose_mat < 0.95) {
        // metal
        auto albedo = color::random(0.5, 1);
        auto fuzz = random_real(0, 0.5);
        sphere_material = std::make_shared<metal>(albedo, fuzz);
      } else {
        // glass
        sphere_material = std::make_shared<glass>(color(1, 1, 1), 1.5);
      }
      bool try_again = false;
      for (const auto& s : spheres) {
        if ((s->center - center).length() < s->radius + radius) {
          try_again = true;
          break;
        }
      }
      if (try_again)
        continue;
      spheres.emplace_back(
        std::make_shared<sphere>(center, radius, sphere_material));
      ++i;
    }
  }

  for (const auto& s : spheres) {
    tracer.add_object(s);
  }
}

int main(int argc, char** argv) {
  uint32_t thread_count = std::thread::hardware_concurrency();
  size_t image_width = 800;
  size_t image_height = 450;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--thread-count") == 0) {
      thread_count = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--width") == 0) {
      image_width = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--height") == 0) {
      image_height = atoi(argv[++i]);
    }
  }

  // Image
  const auto aspect_ratio = static_cast<REAL_T>(image_width) / image_height;
  const auto pixel_count = image_width * image_height;

  InitWindow(image_width, image_height, "RTIOW");

  Image image = LoadImageFromScreen();
  Texture2D tex = LoadTextureFromImage(image);

  auto ground_mat = std::make_shared<lambertian>(color(0.5, 0.5, 0.5));

  camera cam(90, aspect_ratio);
  cam.origin = vec3(13, 2, 3);
  cam.look_at(vec3(0, 0, 0));
  cam.focus_distance = 10;
  ray_tracer tracer(cam, 4, 20, image_width, image_height);
  tracer.add_object(
    std::make_shared<plane>(point3(0, 0, 0), vec3(0, 1, 0), ground_mat));
  scatter_objects(tracer);

  thread_pool pool(thread_count);

  bool done = false;
  REAL_T rt_frame_time = 0;
  std::atomic_uint progress = 0;
  stopwatch rt_sw;
  auto rt_thread = std::thread([&done, &pool, &tracer, &image, &rt_frame_time,
                                &progress, &rt_sw, &pixel_count]() {
    const size_t segments = pool.pool_size();
    const size_t seg_size = image.width / segments;
    std::cout << "Segments: " << segments << std::endl;
    std::cout << "Segment size: " << seg_size << std::endl;
    for (size_t si = 0; si < segments; ++si) {
      pool.enqueue([&image, si, seg_size, segments, &progress, &tracer, &done,
                    &rt_frame_time, &rt_sw, &pixel_count]() {
        const size_t start = si * seg_size;
        const size_t end = si == segments - 1 ? image.width : start + seg_size;
        char buf[256];
        snprintf(buf, 256, "Segment %zu [%zu, %zu)\n", si, start, end);
        std::cout << buf << std::flush;
        while (!done) {
          for (size_t i = start; i < end; ++i) {
            for (size_t j = 0; j < image.height; ++j) {
              REAL_T u = static_cast<REAL_T>(i) / (image.width - 1);
              REAL_T v = static_cast<REAL_T>(j) / (image.height - 1);
              v = 1.0 - v;
              auto c = tracer.compute(u, v);
              write_pixel(image, i, j, c);
              if (progress.fetch_add(1, std::memory_order_relaxed) ==
                  pixel_count - 1) {
                rt_frame_time = rt_sw.elapsed();
                progress.store(0);
                rt_sw.reset();
              }
              if (done) {
                break;
              }
            }
          }
        }
      });
    }
    pool.start();
    pool.wait();
  });

  bool lock_cam = true;

  REAL_T render_fps = 0;
  char filename[256] = "render.png";
  bool editing_filename = false;

  while (!WindowShouldClose()) {
    stopwatch sw;
    BeginDrawing();

    if (!lock_cam) {
      REAL_T move_right = 0;
      REAL_T move_front = 0;
      if (IsKeyDown(KEY_W))
        move_front += 1;
      if (IsKeyDown(KEY_S))
        move_front -= 1;
      if (IsKeyDown(KEY_A))
        move_right -= 1;
      if (IsKeyDown(KEY_D))
        move_right += 1;
      REAL_T move_speed = IsKeyDown(KEY_LEFT_SHIFT) ? 0.1 : 0.01;
      cam.move(move_right * move_speed, move_front * move_speed);

      if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        auto delta = GetMouseDelta();
        REAL_T dx = delta.x / image_width * 2;
        REAL_T dy = delta.y / image_height * 2;
        cam.change_direction(dx, dy);
      }
    }

    UpdateTexture(tex, image.data);
    DrawTexture(tex, 0, 0, WHITE);

    REAL_T rt_fps = 1.0 / rt_frame_time;
    char fps_str[128];
    snprintf(fps_str, sizeof(fps_str), "RT: %.2f FPS (%s)", rt_fps,
             stopwatch::elapsed_str(rt_frame_time).c_str());
    GuiLabel(Rectangle{5, 0, 200, 20}, fps_str);
    snprintf(fps_str, sizeof(fps_str), "Render: %.2f FPS", render_fps);
    GuiLabel(Rectangle{5, 20, 150, 20}, fps_str);

    // Sample count and max depth sliders
    float sample_count = static_cast<float>(tracer.sample_count);
    float max_depth = static_cast<float>(tracer.max_depth);
    GuiSlider(Rectangle{5, 45, 150, 20}, nullptr,
              TextFormat("Samples %i", int(sample_count)), &sample_count, 1,
              1000);
    GuiSlider(Rectangle{5, 70, 150, 20}, nullptr,
              TextFormat("Max Depth %i", int(max_depth)), &max_depth, 1, 100);
    // Camera settings
    const float cam_settings_start = 95;
    GuiCheckBox(Rectangle{5, cam_settings_start, 20, 20}, "Lock camera",
                &lock_cam);
    tracer.sample_count = static_cast<size_t>(sample_count);
    tracer.max_depth = static_cast<size_t>(max_depth);
    GuiSlider(Rectangle{5, cam_settings_start + 25, 150, 20}, nullptr,
              TextFormat("Focus Distance %.2f", cam.focus_distance),
              &cam.focus_distance, 0.1, 10);
    GuiSlider(Rectangle{5, cam_settings_start + 50, 150, 20}, nullptr,
              TextFormat("Aperture %.2f", cam.aperture), &cam.aperture, 0.1,
              10);
    const float cam_settings_end = cam_settings_start + 75;

    // Image settings
    const float img_settings_start = cam_settings_end;
    // Write to disk
    Rectangle filename_textbox_bounds = {5, img_settings_start, 150, 20};
    if (CheckCollisionPointRec(GetMousePosition(), filename_textbox_bounds) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      editing_filename = true;
    }
    if (editing_filename &&
        !CheckCollisionPointRec(GetMousePosition(), filename_textbox_bounds) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      editing_filename = false;
    }
    GuiTextBox(filename_textbox_bounds, filename, 256, editing_filename);
    if (GuiButton(Rectangle{5, img_settings_start + 25, 150, 20}, "Export")) {
      ExportImage(image, filename);
    }
    // Reset image
    if (GuiButton(Rectangle{5, img_settings_start + 50, 150, 20}, "Reset")) {
      pool.cancel();
      for (size_t i = 0; i < image.width * image.height; ++i) {
        write_pixel(image, i % image.width, i / image.width, color(0, 0, 0));
      }
    }

    // Progress bar at the bottom
    float progress_f = static_cast<float>(progress.load());
    constexpr float progress_bar_thickness = 8.f;
    GuiProgressBar(
      Rectangle{0, image.height - progress_bar_thickness,
                static_cast<float>(image.width), progress_bar_thickness},
      0, 0, &progress_f, 0, static_cast<float>(image.width * image.height));
    EndDrawing();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    render_fps = 1.0 / sw.elapsed();
  }

  done = true;
  pool.cancel();
  rt_thread.join();

  UnloadTexture(tex);
  CloseWindow();
  return 0;
}
