#pragma once

// std
#include <vector>
#include <memory>

namespace hnll{

template <typename T> using u_ptr = std::unique_ptr<T>;
template <typename T> using s_ptr = std::shared_ptr<T>;

namespace geometry {

// forward declaration
class bounding_volume;
class perspective_frustum;

namespace intersection {
  bool intersection_bounding_volume(const bounding_volume& a, const bounding_volume& b);
  // testing functions for culling algorithms
  static bool intersection_sphere_frustum(const geometry::bounding_volume& sphere, const perspective_frustum& frustum);
  bool intersection_aabb_aabb      (const bounding_volume& aabb_a, const bounding_volume& aabb_b);
  // TODO : impl
  bool intersection_aabb_sphere    (const bounding_volume& aabb, const bounding_volume& sphere);
  bool intersection_sphere_sphere  (const bounding_volume& sphere_a, const bounding_volume& sphere_b);
}; // namespace intersection

}} // namespace hnll::physics