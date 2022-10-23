#pragma once

// lib
#include <eigen3/Eigen/Dense>

namespace hnll{

using vec3 = Eigen::Vector3d;

namespace game { using actor_id = unsigned int; }

namespace physics {

struct collision_info
{
  double mass;
  vec3   velocity_;
  game::actor_id actor_a_;
  game::actor_id actor_b_;
};

}} // namespace hnll::physics