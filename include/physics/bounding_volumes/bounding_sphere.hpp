#pragma once

// lib
#include <eigen3/Eigen/Dense>

namespace hnll::physics {

enum class ctor_type
{
    AABB_BASED,
    RICHOR
};

class bounding_sphere
{
  public:
    bounding_sphere(const Eigen::Vector3d& center_point, const double radius)
    :center_point_(center_point), radius_(radius){}

    // ctor selector
    static bounding_sphere create_bounding_sphere(ctor_type type, std::vector<Eigen::Vector3f>& vertices);
    // ctor impls
    static bounding_sphere aabb_based_ctor(std::vector<Eigen::Vector3f>& vertices);
    static bounding_sphere richor_ctor(std::vector<Eigen::Vector3f>& vertices);

    // intersection test with other kinds of bounding volumes
    bool intersect_with(const bounding_sphere& other);

    // getter
    const Eigen::Vector3d& get_center_point() const { return center_point_; }
    inline double          get_radius() const{ return radius_; }
    // setter
    void set_center_point(const Eigen::Vector3d& center_point) { center_point_ = center_point; }
    void set_radius(const double radius) { radius_ = radius; }
  private:
    Eigen::Vector3d center_point_;
    double radius_;
};

} // namespace hnll::physics