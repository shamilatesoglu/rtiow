#include "image_texture.h"

#include "raylib.h"

image_texture::image_texture(const char* filename) {
  image = LoadImage(filename);
}

image_texture::image_texture(const char* file_type, const unsigned char* data,
                             int size) {
  image = LoadImageFromMemory(file_type, data, size);
}

color image_texture::value(double u, double v, const vec3& p) const {
  // If we have no texture data, then return solid red as a debugging aid.
  if (image.data == nullptr)
    return color(1, 0, 0);

  // Clamp input texture coordinates to [0,1] x [1,0]
  u = clamp(u, 0.0, 1.0);
  v = 1.0 - clamp(v, 0.0, 1.0);  // Flip V to image coordinates

  auto i = static_cast<int>(u * image.width);
  auto j = static_cast<int>(v * image.height);

  // Clamp integer mapping, since actual coordinates should be less than 1.0
  if (i >= image.width)
    i = image.width - 1;
  if (j >= image.height)
    j = image.height - 1;

  const auto color_scale = 1.0 / 255.0;
  Color pixel = GetImageColor(image, i, j);

  return color(pixel.r * color_scale, pixel.g * color_scale,
               pixel.b * color_scale);
}
