#pragma once

// lib
#include <eigen3/Eigen/Dense>

namespace hnll{

using vec3 = Eigen::Vector3d;

namespace physics {

struct collision_info
{
  double mass;
  vec3   velocity_;
};

}} // namespace hnll::physics