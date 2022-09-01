// hnll
#include <physics/bounding_volumes/bounding_sphere.hpp>

namespace hnll::physics {

bool bounding_sphere::intersection_test(const bounding_sphere &other)
{
  Eigen::Vector3d difference = this->center_point_ - other.get_center_point();
  double distance2 = difference.dot(difference);
  float radius_sum = this->radius_ + other.get_radius();
  return distance2 <= radius_sum;
}

} // namespcae hnll::physics