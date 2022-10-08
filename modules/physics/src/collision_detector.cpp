// hnll
#include <physics/collision_detector.hpp>
#include <physics/rigid_component.hpp>
#include <geometry/bounding_volume.hpp>

// lib
#include <eigen3/Eigen/Dense>

using hnll::geometry::bounding_volume;

namespace hnll::physics {

using point = Eigen::Vector3d;
struct plane
{
  Eigen::Vector3d point;
  Eigen::Vector3d normal;
  // plane's normal is guaranteed to be normalized
  plane(const Eigen::Vector3d& point_, const Eigen::Vector3d& normal_) : point(point_), normal(normal_) { normal.normalize(); }
};

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

bool collision_detector::intersection_sphere_sphere(const bounding_volume &sphere_a, const bounding_volume &sphere_b)
{
  Eigen::Vector3d difference = sphere_a.get_center_point() - sphere_b.get_center_point();
  double distance2 = difference.dot(difference);
  float radius_sum = sphere_a.get_sphere_radius() + sphere_b.get_sphere_radius();
  return distance2 <= radius_sum * radius_sum;
}

// support functions for intersection_aabb_sphere
// prefix 'cp' is abbreviation of 'closest point'
point cp_point_to_plane(const point& q, const plane& p)
{
  // plane's normal must be normalized before this test
  float t = p.normal.dot(q - p.point);
  return q - t * p.normal;
}

double distance_point_to_plane(const point& q, const plane& p)
{
  return p.normal.dot(q - p.point);
}

// caller of this function is responsible for insuring that the bounding_volume is aabb
point cp_point_to_aabb(const point& p, const bounding_volume& aabb)
{
  point q;
  // TODO : simdlize
  for (int i = 0; i < 3; i++){
    float v = p[i];
    if (v < aabb.get_center_point()[i] - aabb.get_aabb_radius()[i]) v = aabb.get_center_point()[i] - aabb.get_aabb_radius()[i];
    else if (v > aabb.get_center_point()[i] + aabb.get_aabb_radius()[i]) v = aabb.get_center_point()[i] + aabb.get_aabb_radius()[i];
    q[i] = v;
  }
  return q;
}

// sq_dist is abbreviation of 'squared distance'
double sq_dist_point_to_aabb(const point& p, const bounding_volume& aabb)
{
  double result = 0.0f;
  for (int i = 0; i < 3; i++) {
    float v = p[i];
    if (v < aabb.get_center_point()[i] - aabb.get_aabb_radius()[i]) result += std::pow(aabb.get_center_point()[i] - aabb.get_aabb_radius()[i] - v, 2);
    else if (v > aabb.get_center_point()[i] + aabb.get_aabb_radius()[i]) result += std::pow(v - aabb.get_center_point()[i] - aabb.get_aabb_radius()[i], 2);
  }
  return result;
}

bool collision_detector::intersection_aabb_sphere(const bounding_volume &aabb, const bounding_volume &sphere)
{
  auto sq_dist = sq_dist_point_to_aabb(sphere.get_center_point(), aabb);
  return std::pow(sphere.get_sphere_radius(), 2) > sq_dist;
}

} // namespace hnll::physics