#include "material.h"

#include "ray.h"

bool lambertian::scatter(const ray& r_in,
                         const hit_record& rec,
                         color& attenuation,
                         ray& scattered) const {
  auto scatter_direction = rec.normal + random_unit_vector();
  // Catch degenerate scatter direction
  if (scatter_direction.near_zero()) {
    scatter_direction = rec.normal;
  }
  scattered = ray(rec.p, scatter_direction);
  attenuation = albedo;
  return true;
}

bool metal::scatter(const ray& r_in,
                    const hit_record& rec,
                    color& attenuation,
                    ray& scattered) const {
  vec3 reflected = reflect(r_in.direction().normalized(), rec.normal);
  scattered = ray(rec.p, reflected + fuzz * random_unit_vector());
  attenuation = albedo;
  return (scattered.direction().dot(rec.normal) > 0);
}

bool glass::scatter(const ray& r_in,
                          const hit_record& rec,
                          color& attenuation,
                          ray& scattered) const {
  attenuation = albedo;
  REAL_T refraction_ratio = rec.front_face ? (1.0 / ior) : ior;
  vec3 unit_direction = r_in.direction().normalized();
  double cos_theta = fmin((-unit_direction).dot(rec.normal), 1.0);
  double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

  bool cannot_refract = refraction_ratio * sin_theta > 1.0;
  vec3 direction;

  if (cannot_refract)
    direction = reflect(unit_direction, rec.normal);
  else
    direction = refract(unit_direction, rec.normal, refraction_ratio);

  scattered = ray(rec.p, direction);
  return true;
}