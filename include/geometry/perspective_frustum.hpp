#pragma once

// lib
#include <eigen3/Eigen/Dense>

namespace hnll {

using vec3 = Eigen::Vector3d;
// forward declaration
namespace utils { class transform; }

namespace geometry {

class perspective_frustum
{
  public:
    void update_plane_normals(const utils::transform& tf);

    // getter
    vec3 get_near_n() const   { return near_n_; }
    vec3 get_far_n() const    { return far_n_; }
    vec3 get_left_n() const   { return left_n_; }
    vec3 get_right_n() const  { return right_n_; }
    vec3 get_top_n() const    { return top_n_; }
    vec3 get_bottom_n() const { return bottom_n_; }
  private:
    vec3   near_n_, far_n_, left_n_, right_n_, top_n_, bottom_n_;
    double fov_x_, fov_y_, near_, far_;
};


}} // namespace hnll::geometry
