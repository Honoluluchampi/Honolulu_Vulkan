#pragma once

// lib
#include <eigen3/Eigen/Dense>

namespace hnll::physics {

class aabb
{
  public:
    aabb();
  private:
    Eigen::Vector3d center_point_;
    Eigen::Vector3d radius_;
};

} // namespace hnll::physics