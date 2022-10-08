#pragma once

// std
#include <memory>

// lib
#include <eigen3/Eigen/Dense>

namespace hnll {

template<typename T> using s_ptr = std::shared_ptr<T>;
template<typename T> using u_ptr = std::unique_ptr<T>;
using vec3 = Eigen::Vector3d;
// forward declaration
namespace utils { class transform; }

namespace geometry {

struct plane
{
  vec3 point;
  vec3 normal;
  // plane's normal is guaranteed to be normalized
  plane(const vec3& point_, const vec3& normal_) : point(point_), normal(normal_) { normal.normalize(); }
  plane() : normal({0.f, -1.f, 0.f}) {}
};

class perspective_frustum
{
  public:
    void update_planes(const utils::transform& tf);

    // getter
    vec3 get_near_n()   const { return near_->normal; }
    vec3 get_far_n()    const { return far_->normal; }
    vec3 get_left_n()   const { return left_->normal; }
    vec3 get_right_n()  const { return right_->normal; }
    vec3 get_top_n()    const { return top_->normal; }
    vec3 get_bottom_n() const { return bottom_->normal; }
    const plane& get_near_ref()   const { return *near_; }
    const plane& get_far_ref()    const { return *far_; }
    const plane& get_left_ref()   const { return *left_; }
    const plane& get_right_ref()  const { return *right_; }
    const plane& get_top_ref()    const { return *top_; }
    const plane& get_bottom_ref() const { return *bottom_; }
  private:
    u_ptr<plane>  near_, far_, left_, right_, top_, bottom_;
    double fov_x_ = M_PI / 4.f, fov_y_ = M_PI / 4.f, near_z_, far_z_;
};


}} // namespace hnll::geometry
