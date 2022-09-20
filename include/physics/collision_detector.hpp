#pragma once

// std
#include <vector>

namespace hnll::physics {

// forward declaration
class bounding_volume;

class collision_detector
{
  public:
    bool intersection_test(const bounding_volume& a, const bounding_volume& b);

  private:
    bool intersection_aabb_aabb(const bounding_volume& aabb_a, const bounding_volume& aabb_b);
    // TODO : impl
    bool intersection_aabb_sphere(const bounding_volume& aabb, const bounding_volume& sphere);
    bool intersection_sphere_sphere(const bounding_volume& sphere_a, const bounding_volume& sphere_b);

    std::vector<bounding_volume> bounding_volumes_{};
};

} // namespace hnll::physics