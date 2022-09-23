#pragma once

// std
#include <vector>
#include <memory>

template <typename T> using u_ptr = std::unique_ptr<T>;
template <typename T> using s_ptr = std::shared_ptr<T>;

namespace hnll::physics {

// forward declaration
class rigid_component;
class bounding_volume;

class collision_detector
{
  public:
    bool intersection_bounding_volume(const bounding_volume& a, const bounding_volume& b);
    static void add_rigid_component(const s_ptr<rigid_component>& rc) { rigid_components_.push_back(rc); }

  private:
    bool intersection_aabb_aabb(const bounding_volume& aabb_a, const bounding_volume& aabb_b);
    // TODO : impl
    bool intersection_aabb_sphere(const bounding_volume& aabb, const bounding_volume& sphere);
    bool intersection_sphere_sphere(const bounding_volume& sphere_a, const bounding_volume& sphere_b);

    static std::vector<s_ptr<rigid_component>> rigid_components_;
};

} // namespace hnll::physics