#pragma once

// lib
#include <eigen3/Eigen/Dense>

namespace hnll::physics {

enum class ctor_type
{
  RITTER
};

class bounding_sphere
{
  public:
    bounding_sphere(const Eigen::Vector3d& center_point = {0.f, 0.f, 0.f}, const double radius = 1.f)
    :center_point_(center_point), radius_(radius){}

    // ctor selector
    static bounding_sphere create_bounding_sphere(ctor_type type, std::vector<Eigen::Vector3d>& vertices);
    // ctor impls
    static bounding_sphere ritter_ctor(std::vector<Eigen::Vector3d>& vertices);

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

// support functions
std::pair<int,int> most_separated_points_on_aabb(const std::vector<Eigen::Vector3d> &vertices);
bounding_sphere sphere_from_distant_points(const std::vector<Eigen::Vector3d> &vertices);
void extend_sphere_to_point(bounding_sphere& sphere, const Eigen::Vector3d& point);

} // namespace hnll::physics