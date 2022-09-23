// hnll
#include <physics/collision_detector.hpp>
#include <physics/rigid_component.hpp>
#include <physics/bounding_volume.hpp>

// lib
#include <eigen3/Eigen/Dense>

namespace hnll::physics {

// static members' declaration
std::vector<s_ptr<rigid_component>> rigid_components_{};

bool collision_detector::intersection_bounding_volume(const bounding_volume &a, const bounding_volume &b)
{
  // call intersection test depending on the types of bv
  if (a.is_aabb() && b.is_aabb())     return intersection_aabb_aabb(a, b);
  if (a.is_aabb() && b.is_sphere())   return intersection_aabb_sphere(a, b);
  if (a.is_sphere() && b.is_aabb())   return intersection_aabb_sphere(b, a);
  if (a.is_sphere() && b.is_sphere()) return intersection_sphere_sphere(a, b);

  std::runtime_error("invalid bounding_volume pair");
  return false;
}

bool collision_detector::intersection_aabb_aabb(const bounding_volume &aabb_a, const bounding_volume &aabb_b)
{
  if (std::abs(aabb_a.get_center_point().x() - aabb_b.get_center_point().x()) > aabb_a.get_aabb_radius().x() + aabb_b.get_aabb_radius().x()) return false;
  if (std::abs(aabb_a.get_center_point().y() - aabb_b.get_center_point().y()) > aabb_a.get_aabb_radius().y() + aabb_b.get_aabb_radius().y()) return false;
  if (std::abs(aabb_a.get_center_point().z() - aabb_b.get_center_point().z()) > aabb_a.get_aabb_radius().z() + aabb_b.get_aabb_radius().z()) return false;
  return true;
}

bool collision_detector::intersection_aabb_sphere(const bounding_volume &aabb, const bounding_volume &sphere)
{
  return true;
}

bool collision_detector::intersection_sphere_sphere(const bounding_volume &sphere_a, const bounding_volume &sphere_b)
{
  Eigen::Vector3d difference = sphere_a.get_center_point() - sphere_b.get_center_point();
  double distance2 = difference.dot(difference);
  float radius_sum = sphere_a.get_sphere_radius() + sphere_b.get_sphere_radius();
  return distance2 <= radius_sum * radius_sum;
}

} // namespace hnll::physics