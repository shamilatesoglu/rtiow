#pragma once

#include "texture.h"

#include "raylib.h"

class image_texture : public texture {
 public:
  const static int bytes_per_pixel = 3;

  image_texture() : image() {}

  image_texture(const char* filename);
  image_texture(const char* file_type, const unsigned char* data, int size);

  ~image_texture() {
    if (image.data)
      UnloadImage(image);
  }

  virtual color value(double u, double v, const vec3& p) const override;

 protected:
  Image image = {0};
};