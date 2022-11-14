#pragma once

// lib
#include <eigen3/Eigen/Dense>

namespace hnll{

using vec3d = Eigen::Vector3d;

namespace game { using actor_id = unsigned int; }

namespace physics {

struct collision_info
{
  double mass;
  vec3d  velocity;
  double intersection_depth;
  game::actor_id actor_a;
  game::actor_id actor_b;
};

}} // namespace hnll::physics