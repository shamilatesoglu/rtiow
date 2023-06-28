#include <iostream>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "aabb.h"
#include "bvh.h"
#include "camera.h"
#include "draw.h"
#include "object.h"
#include "ray.h"
#include "ray_tracer.h"
#include "raylib.h"
#include "stopwatch.h"
#include "thread_pool.h"
#include "vec.h"

void scatter_objects(ray_tracer& tracer) {
  std::vector<std::shared_ptr<hittable>> objects;
  // Place 3 big spheres
  auto material1 = std::make_shared<glass>(color(1, 1, 1), 1.5);
  objects.emplace_back(
    std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

  auto material2 = std::make_shared<lambertian>(color(0.4, 0.2, 0.1));
  objects.emplace_back(
    std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

  auto material3 = std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
  objects.emplace_back(
    std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

  for (size_t i = 0; i < 800;) {
    auto choose_mat = random_real();
    auto radius = random_real(0.05, 0.25);
    auto center = vec3(random_real(-10, 10), radius, random_real(-10, 10));

    std::shared_ptr<material> sphere_material;
    std::shared_ptr<hittable> candidate;

    if (choose_mat < 0.8) {
      // diffuse
      auto albedo = color::random() * color::random();
      sphere_material = std::make_shared<lambertian>(albedo);
      auto center2 = center + vec3(0, random_real(0, 0.5), 0);
      candidate = std::make_shared<moving_sphere>(center, center2, 0.0, 1.0,
                                                  radius, sphere_material);
    } else if (choose_mat < 0.95) {
      // metal
      auto albedo = color::random(0.5, 1);
      auto fuzz = random_real(0, 0.5);
      sphere_material = std::make_shared<metal>(albedo, fuzz);
      candidate = std::make_shared<sphere>(center, radius, sphere_material);
    } else {
      // glass
      sphere_material = std::make_shared<glass>(color(1, 1, 1), 1.5);
      candidate = std::make_shared<sphere>(center, radius, sphere_material);
    }
    bool try_again = false;
    for (const auto& obj : objects) {
      aabb box1;
      aabb box2;
      if (!obj->bounding_box(0, 1, box1) ||
          !candidate->bounding_box(0, 1, box2)) {
        continue;
      }
      if (box1.overlaps(box2)) {
        try_again = true;
        break;
      }
    }
    if (!try_again) {
      objects.emplace_back(candidate);
      ++i;
    }
  }

  for (const auto& s : objects) {
    tracer.add_object(s);
  }
}

int main(int argc, char** argv) {
  uint32_t thread_count = std::thread::hardware_concurrency() - 1;
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
  const auto aspect_ratio = static_cast<real_t>(image_width) / image_height;
  const auto pixel_count = image_width * image_height;

  InitWindow(image_width, image_height, "RTIOW");

  Image image = LoadImageFromScreen();
  Texture2D tex = LoadTextureFromImage(image);

  auto ground_mat = std::make_shared<lambertian>(std::make_shared<plane_checker_texture>(color(0,0,0), color(1,1,1)));

  camera cam(90, aspect_ratio, 0.0, 10, point3(13, 2, 3), 0, 1);
  cam.look_at(vec3(0, 0, 0));

  ray_tracer tracer(cam, 1, 50, image_width, image_height);
  tracer.add_object(
    std::make_shared<plane>(point3(0, 0, 0), vec3(0, 1, 0), ground_mat));
  scatter_objects(tracer);

  tracer.build_bvh();

  thread_pool pool(thread_count);

  bool done = false;
  bool suspend = false;
  real_t rt_frame_time = 0;
  std::atomic_uint progress = 0;
  stopwatch rt_sw;
  auto rt_thread = std::thread([&done, &suspend, &pool, &tracer, &image,
                                &rt_frame_time, &progress, &rt_sw,
                                &pixel_count]() {
    const size_t segments = pool.pool_size();
    const size_t seg_size = image.width / segments;
    std::cout << "Segments: " << segments << std::endl;
    std::cout << "Segment size: " << seg_size << std::endl;
    for (size_t si = 0; si < segments; ++si) {
      pool.enqueue([&image, si, seg_size, segments, &progress, &tracer, &done,
                    &suspend, &rt_frame_time, &rt_sw, &pixel_count]() {
        const size_t start = si * seg_size;
        const size_t end = si == segments - 1 ? image.width : start + seg_size;
        char buf[256];
        snprintf(buf, 256, "Segment %zu [%zu, %zu)\n", si, start, end);
        std::cout << buf << std::flush;
        while (!done) {
          for (size_t i = start; i < end; ++i) {
            for (size_t j = 0; j < image.height; ++j) {
              tracer.render_pixel(image, i, j);
              if (progress.fetch_add(1, std::memory_order_relaxed) ==
                  pixel_count - 1) {
                rt_frame_time = rt_sw.elapsed();
                progress.store(0);
                rt_sw.reset();
              }
              if (done) {
                break;
              }
              while (suspend) {
                std::this_thread::sleep_for(std::chrono::milliseconds(400));
              }
            }
          }
        }
      });
    }
    pool.start();
    pool.wait();
  });

  bool debug = false;
  bool lock_cam = false;
  real_t render_fps = 0;
  char filename[256] = "render.png";
  bool editing_filename = false;
  stopwatch progress_sw;
  while (!WindowShouldClose()) {
    stopwatch sw;
    BeginDrawing();

    if (!lock_cam) {
      real_t move_right = 0;
      real_t move_front = 0;
      if (IsKeyDown(KEY_W))
        move_front += 1;
      if (IsKeyDown(KEY_S))
        move_front -= 1;
      if (IsKeyDown(KEY_A))
        move_right -= 1;
      if (IsKeyDown(KEY_D))
        move_right += 1;
      real_t move_speed = IsKeyDown(KEY_LEFT_SHIFT) ? 0.1 : 0.01;
      cam.move(move_right * move_speed, move_front * move_speed);

      if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        auto delta = GetMouseDelta();
        real_t dx = delta.x / image_width * 2;
        real_t dy = delta.y / image_height * 2;
        cam.change_direction(dx, dy);
      }
    }

    if (IsKeyPressed(KEY_F1)) {
      debug = !debug;
    }
    if (debug) {
      draw_bvh(image, tracer.bvh_root.get(), tracer, 0);
    }
    UpdateTexture(tex, image.data);
    DrawTexture(tex, 0, 0, WHITE);
    
    real_t rt_fps = 1.0 / rt_frame_time;
    float progress_f = static_cast<float>(progress.load());
    float max_progress = static_cast<float>(image.width * image.height);
    // Calculate the rate of progress per second
    static real_t progress_per_sec = 0;
    if (progress_sw.elapsed() >= 1.0) {
        progress_sw.reset();
        static real_t progress_last = 0;
        progress_per_sec = progress_f - progress_last;
        progress_last = progress_f;
    }
    // Remaining time
    real_t remaining = (max_progress - progress_f) / progress_per_sec;

    char perf_str[128];
    snprintf(perf_str, sizeof(perf_str), "RT: %.2f FPS (%s) Rem: %s", rt_fps,
             stopwatch::elapsed_str(rt_frame_time).c_str(), stopwatch::elapsed_str(remaining).c_str());
    GuiLabel(Rectangle{5, 0, 280, 20}, perf_str);
    snprintf(perf_str, sizeof(perf_str), "Render: %.2f FPS", render_fps);
    GuiLabel(Rectangle{5, 20, 150, 20}, perf_str);

    // Camera position (top right corner)
    GuiLabel(Rectangle{image_width - 200.f, 0, 200, 20},
             TextFormat("Camera: (%.2f, %.2f, %.2f)", cam.origin.x(),
                        cam.origin.y(), cam.origin.z()));

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
              &cam.focus_distance, 0.5, 50);
    GuiSlider(Rectangle{5, cam_settings_start + 50, 150, 20}, nullptr,
              TextFormat("Aperture %.2f", cam.aperture), &cam.aperture, 0.001,
              2.0);
    const float cam_settings_end = cam_settings_start + 75;

    // Image settings
    const float img_settings_start = cam_settings_end;
    const char* mode_str = "Color;Normal;Depth";
    GuiComboBox(Rectangle{5, img_settings_start, 150, 20}, mode_str,
                reinterpret_cast<int*>(&tracer.mode));
    // Write to disk
    Rectangle filename_textbox_bounds = {5, img_settings_start + 25, 150, 20};
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
    if (GuiButton(Rectangle{5, img_settings_start + 50, 150, 20}, "Export")) {
      ExportImage(image, filename);
    }
    // Reset image
    if (GuiButton(Rectangle{5, img_settings_start + 75, 150, 20}, "Reset")) {
      pool.cancel();
      for (size_t i = 0; i < image.width * image.height; ++i) {
        write_pixel(image, i % image.width, i / image.width, color(0, 0, 0));
      }
    }
    if (GuiButton(Rectangle{5, img_settings_start + 100, 150, 20},
                  suspend ? "Resume" : "Suspend")) {
      suspend = !suspend;
    }
    if (debug) {
      // With red text
      int old_color = GuiGetStyle(LABEL, TEXT_COLOR_NORMAL);
      GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, 0xffff0000);
      GuiLabel(Rectangle{5, image.height - 28.f, 150, 20}, TextFormat("DEBUG"));
      GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, old_color);
    }

    // Progress bar at the bottom
    constexpr float progress_bar_thickness = 8.f;
    GuiProgressBar(
      Rectangle{0, image.height - progress_bar_thickness,
                static_cast<float>(image.width), progress_bar_thickness},
      0, 0, &progress_f, 0, max_progress);
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
