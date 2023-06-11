#include <iostream>

#include "raylib.h"

#include "vec.h"

int main(void) {
  const int image_width = 256;
  const int image_height = 256;

  InitWindow(image_width, image_height, "RTIOW");

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    for (int j = image_height - 1; j >= 0; --j) {
      for (int i = 0; i < image_width; ++i) {
        auto r = double(i) / (image_width - 1);
        auto g = double(j) / (image_height - 1);
        auto b = 0.25;

        auto ir = static_cast<uint8_t>(255.999 * r);
        auto ig = static_cast<uint8_t>(255.999 * g);
        auto ib = static_cast<uint8_t>(255.999 * b);

        DrawPixel(i, j, Color{ir, ig, ib, 255});
      }
    }
    EndDrawing();
  }

  CloseWindow();

  return 0;
}