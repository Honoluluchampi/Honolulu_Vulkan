// hnll
#include <physics/bounding_volumes/bounding_sphere.hpp>

namespace hnll::physics {

bounding_sphere bounding_sphere::create_bounding_sphere(ctor_type type, std::vector<Eigen::Vector3f> &vertices)
{
  switch (type) {
    case ctor_type::AABB_BASED:
      return aabb_based_ctor(vertices);
    case ctor_type::RICHOR:
      return richor_ctor(vertices);
    default:
      std::runtime_error("invalid bounding-sphere-ctor type");
  }
}

bounding_sphere bounding_sphere::aabb_based_ctor(std::vector<Eigen::Vector3f> &vertices)
{

}

bounding_sphere bounding_sphere::richor_ctor(std::vector<Eigen::Vector3f> &vertices)
{

}

bool bounding_sphere::intersect_with(const bounding_sphere &other)
{
  Eigen::Vector3d difference = this->center_point_ - other.get_center_point();
  double distance2 = difference.dot(difference);
  float radius_sum = this->radius_ + other.get_radius();
  return distance2 <= radius_sum * radius_sum;
}

} // namespcae hnll::physics