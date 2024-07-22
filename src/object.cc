#include "object.h"
#include "bvh.h"
#include "common.h"

#include <cfloat>
#include <cmath>
#include <mutex>
#include <unordered_set>

void get_sphere_uv(const vec3& p, real_t& u, real_t& v) {
  // p: point on unit sphere
  // u: [0, 1] longitude
  // v: [0, 1] latitude

  auto theta = acos(-p.y());
  auto phi = atan2(-p.z(), p.x()) + M_PI;
  u = phi / (2 * M_PI);
  v = theta / M_PI;
}

void get_plane_uv(const vec3& p, real_t& u, real_t& v) {
  u = p.x();
  v = p.z();
}

bool hittable_list::hit(const ray &r, real_t t_min, real_t t_max, hit_record &rec) const {
  hit_record cur_rec;
  bool hit_anything = false;
  auto closest_so_far = t_max;
  for (const auto& obj : objects) {
    if (obj->hit(r, t_min, closest_so_far, cur_rec)) {
      hit_anything = true;
      closest_so_far = cur_rec.t;
      rec = cur_rec;
    }
  }
  return hit_anything;
}

bool hittable_list::bounding_box(real_t time0, real_t time1,
                                 aabb& output_box) const {
  if (objects.empty())
    return false;

  aabb temp_box;
  bool first_box = true;

  for (const auto& object : objects) {
    if (!object->bounding_box(time0, time1, temp_box))
      return false;
    output_box = first_box ? temp_box : output_box.surrounding(temp_box);
    first_box = false;
  }

  return true;
}

void hittable_list::build_bvh() {
  stopwatch sw;
  bvh_root = bvh_node::build(objects, 0, 1);
  std::cout << "BVH build time: " << sw.elapsed() << "s\n";
}

bool sphere::hit(const ray& r, real_t t_min, real_t t_max,
                 hit_record& rec) const {
  // Ray Center: R
  // Sphere Center: S
  // Day Direction: d
  // Ray Eq: R(t) = R + td
  // ((t^2)d ⋅ d) + (2td ⋅ (R−S)) + ((R−S) ⋅ (R−S)) − r^2 = 0
  const auto& R = r.origin();
  const auto& S = center;
  const auto& d = r.direction();
  // D: From sphere center to ray origin
  auto SR = R - S;
  // At^2 + Bt + C = 0, solve for t
  auto A = d.dot(d);
  auto B = 2 * d.dot(SR);
  auto C = SR.dot(SR) - radius * radius;
  auto discriminant = B * B - 4 * A * C;
  if (discriminant < 0) {
    return false;
  }
  auto t = (-B - sqrt(discriminant)) / (2 * A);
  if (t < t_min || t > t_max) {
    return false;
  }
  rec.t = t;
  rec.p = r.at(t);
  vec3 outward_normal = (rec.p - S) / radius;
  rec.set_face_normal(r, outward_normal);
  get_sphere_uv(outward_normal, rec.u, rec.v);
  rec.mat = mat.get();
  return true;
}

bool sphere::bounding_box(real_t time0, real_t time1, aabb& out) const {
  out = aabb(center - vec3(radius, radius, radius),
             center + vec3(radius, radius, radius));
  return true;
}

bool moving_sphere::hit(const ray& r, real_t t_min, real_t t_max,
                        hit_record& rec) const {
  // Ray Center: R
  // Sphere Center: S
  // Day Direction: d
  // Ray Eq: R(t) = R + td
  // ((t^2)d ⋅ d) + (2td ⋅ (R−S)) + ((R−S) ⋅ (R−S)) − r^2 = 0
  const auto& R = r.origin();
  const auto& S = center(r.time());
  const auto& d = r.direction();
  // D: From sphere center to ray origin
  auto SR = R - S;
  // At^2 + Bt + C = 0, solve for t
  auto A = d.dot(d);
  auto B = 2 * d.dot(SR);
  auto C = SR.dot(SR) - radius * radius;
  auto discriminant = B * B - 4 * A * C;
  if (discriminant < 0) {
    return false;
  }
  auto t = (-B - sqrt(discriminant)) / (2 * A);
  if (t < t_min || t > t_max) {
    return false;
  }
  rec.t = t;
  rec.p = r.at(t);
  vec3 outward_normal = (rec.p - S) / radius;
  rec.set_face_normal(r, outward_normal);
  get_sphere_uv(outward_normal, rec.u, rec.v);
  rec.mat = mat.get();
  return true;
}

bool moving_sphere::bounding_box(real_t time0, real_t time1, aabb& out) const {
  out = (aabb(center(time0) - vec3(radius, radius, radius),
              center(time0) + vec3(radius, radius, radius))
           .surrounding(aabb(center(time1) - vec3(radius, radius, radius),
                             center(time1) + vec3(radius, radius, radius))));
  return true;
}

bool plane::hit(const ray& r, real_t t_min, real_t t_max,
                hit_record& rec) const {
  // Ray Center: R
  // Plane Center: S
  // Day Direction: d
  // Ray Eq: R(t) = R + td
  // (S - R) ⋅ n = 0
  // (S - (R + td)) ⋅ n = 0
  // (S - R) ⋅ n - t(d ⋅ n) = 0
  // t = ((S - R) ⋅ n) / (d ⋅ n)
  const auto& R = r.origin();
  const auto& S = center;
  const auto& d = r.direction();
  auto n = normal;
  auto SR = S - R;
  auto t = SR.dot(n) / d.dot(n);
  if (t < t_min || t > t_max) {
    return false;
  }
  rec.t = t;
  rec.p = r.at(t);
  rec.set_face_normal(r, n);
  get_plane_uv(rec.p, rec.u, rec.v);
  rec.mat = mat.get();
  return true;
}

bool plane::bounding_box(real_t time0, real_t time1, aabb& out) const {
  return false;
}

bool xy_rect::hit(const ray& r, real_t t_min, real_t t_max,
                  hit_record& rec) const {
  auto t = (k - r.origin().z()) / r.direction().z();
  if (t < t_min || t > t_max)
    return false;
  auto x = r.origin().x() + t * r.direction().x();
  auto y = r.origin().y() + t * r.direction().y();
  if (x < x0 || x > x1 || y < y0 || y > y1)
    return false;
  rec.u = (x - x0) / (x1 - x0);
  rec.v = (y - y0) / (y1 - y0);
  rec.t = t;
  auto outward_normal = vec3(0, 0, 1);
  rec.set_face_normal(r, outward_normal);
  rec.mat = this->mat.get();
  rec.p = r.at(t);
  return true;
}

bool xy_rect::bounding_box(real_t time0, real_t time1, aabb& out) const {
  out =
    aabb(vec3(x0, y0, k - 0.0001), vec3(x1, y1, k + 0.0001));  // Pad a little
  return true;
}

bool xz_rect::hit(const ray& r, real_t t_min, real_t t_max,
                  hit_record& rec) const {
  auto t = (k - r.origin().y()) / r.direction().y();
  if (t < t_min || t > t_max)
    return false;
  auto x = r.origin().x() + t * r.direction().x();
  auto z = r.origin().z() + t * r.direction().z();
  if (x < x0 || x > x1 || z < z0 || z > z1)
    return false;
  rec.u = (x - x0) / (x1 - x0);
  rec.v = (z - z0) / (z1 - z0);
  rec.t = t;
  auto outward_normal = vec3(0, 1, 0);
  rec.set_face_normal(r, outward_normal);
  rec.mat = mat.get();
  rec.p = r.at(t);
  return true;
}

bool xz_rect::bounding_box(real_t time0, real_t time1, aabb& output_box) const {
  output_box = aabb(point3(x0, k - 0.0001, z0),
                    point3(x1, k + 0.0001, z1));  // Pad a little
  return true;
}

bool yz_rect::hit(const ray& r, real_t t_min, real_t t_max,
                  hit_record& rec) const {
  auto t = (k - r.origin().x()) / r.direction().x();
  if (t < t_min || t > t_max)
    return false;
  auto y = r.origin().y() + t * r.direction().y();
  auto z = r.origin().z() + t * r.direction().z();
  if (y < y0 || y > y1 || z < z0 || z > z1)
    return false;
  rec.u = (y - y0) / (y1 - y0);
  rec.v = (z - z0) / (z1 - z0);
  rec.t = t;
  auto outward_normal = vec3(1, 0, 0);
  rec.set_face_normal(r, outward_normal);
  rec.mat = mat.get();
  rec.p = r.at(t);
  return true;
}

bool yz_rect::bounding_box(real_t time0, real_t time1, aabb& output_box) const {
  output_box = aabb(point3(k - 0.0001, y0, z0),
                    point3(k + 0.0001, y1, z1));  // Pad a little
  return true;
}

// Transforms

bool translate::hit(const ray& r, real_t t_min, real_t t_max,
                    hit_record& rec) const {
  ray moved_r(r.origin() - offset, r.direction(), r.time());
  if (!ptr->hit(moved_r, t_min, t_max, rec))
    return false;

  rec.p += offset;
  rec.set_face_normal(moved_r, rec.normal);

  return true;
}

bool translate::bounding_box(real_t time0, real_t time1,
                             aabb& output_box) const {
  if (!ptr->bounding_box(time0, time1, output_box))
    return false;

  output_box = aabb(output_box.min + offset, output_box.max + offset);

  return true;
}

rotate_y::rotate_y(std::shared_ptr<hittable> p, real_t angle) : ptr(p) {
  auto radians = deg2rad(angle);
  sin_theta = sin(radians);
  cos_theta = cos(radians);
  has_box = ptr->bounding_box(0, 1, bbox);

  point3 min(INFINITY, INFINITY, INFINITY);
  point3 max(-INFINITY, -INFINITY, -INFINITY);

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        auto x = i * bbox.max.x() + (1 - i) * bbox.min.x();
        auto y = j * bbox.max.y() + (1 - j) * bbox.min.y();
        auto z = k * bbox.max.z() + (1 - k) * bbox.min.z();

        auto newx = cos_theta * x + sin_theta * z;
        auto newz = -sin_theta * x + cos_theta * z;

        vec3 tester(newx, y, newz);

        for (int c = 0; c < 3; c++) {
          min[c] = fmin(min[c], tester[c]);
          max[c] = fmax(max[c], tester[c]);
        }
      }
    }
  }

  bbox = aabb(min, max);
}

bool rotate_y::bounding_box(real_t time0, real_t time1,
                            aabb& output_box) const {
  output_box = bbox;
  return has_box;
}

bool rotate_y::hit(const ray& r, real_t t_min, real_t t_max,
                   hit_record& rec) const {
  auto origin = r.origin();
  auto direction = r.direction();

  origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
  origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

  direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
  direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

  ray rotated_r(origin, direction, r.time());

  if (!ptr->hit(rotated_r, t_min, t_max, rec))
    return false;

  auto p = rec.p;
  auto normal = rec.normal;

  p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
  p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

  normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
  normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

  rec.p = p;
  rec.set_face_normal(rotated_r, normal);

  return true;
}
