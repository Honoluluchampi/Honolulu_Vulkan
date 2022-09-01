#pragma once

// lib
#include <eigen3/Eigen/Dense>

namespace hnll::physics {

class bounding_sphere
{
  public:
    bounding_sphere(const Eigen::Vector3d& center_point, const double radius)
    :center_point_(center_point), radius_(radius){}

    // intersection test with other kinds of bounding volumes
    bool intersection_test(const bounding_sphere& other);

    // getter
    inline Eigen::Vector3d get_center_point() const { return center_point_; }
    inline double          get_radius() const{ return radius_; }
    // setter
    void set_center_point(const Eigen::Vector3d& center_point) { center_point_ = center_point; }
    void set_radius(const double radius) { radius_ = radius; }
  private:
    Eigen::Vector3d center_point_;
    double radius_;
};

} // namespace hnll::physics