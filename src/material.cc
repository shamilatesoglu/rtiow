#include "material.h"

#include "ray.h"

bool lambertian::scatter(const ray &r_in, const hit_record &rec,
                         color &attenuation, ray &scattered) const {
  auto scatter_direction = rec.normal + random_unit_vector();
  // Catch degenerate scatter direction
  if (scatter_direction.near_zero()) {
    scatter_direction = rec.normal;
  }
  scattered = ray(rec.p, scatter_direction);
  attenuation = albedo;
  return true;
}

bool metal::scatter(const ray &r_in, const hit_record &rec, color &attenuation,
                    ray &scattered) const {
  vec3 reflected = reflect(r_in.direction().normalized(), rec.normal);
  scattered = ray(rec.p, reflected + fuzz * random_unit_vector());
  attenuation = albedo;
  return (scattered.direction().dot(rec.normal) > 0);
}