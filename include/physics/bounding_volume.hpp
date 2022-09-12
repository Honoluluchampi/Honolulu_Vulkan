#pragma once

// lib
#include <eigen3/Eigen/Dense>

namespace hnll::physics {

// bounding volume type
enum class bv_type {
    SPHERE,
    AABB
};

enum class bv_ctor_type {
    RITTER,
};

class bounding_volume
{
  public:
    // ctor for aabb
    bounding_volume(const Eigen::Vector3d& center_point = {0.f, 0.f, 0.f}, const Eigen::Vector3d radius = {0.f, 0.f, 0.f})
    :center_point_(center_point), radius_(radius), bv_type_(bv_type::AABB){}
    // ctor for sphere
    bounding_volume(const Eigen::Vector3d& center_point = {0.f, 0.f, 0.f}, const double radius = 1.f)
    :center_point_(center_point), radius_(radius, 0.f, 0.f), bv_type_(bv_type::SPHERE){}

    static bounding_volume create_aabb(std::vector<Eigen::Vector3d>& vertices);
    static bounding_volume create_bounding_sphere(bv_ctor_type type, std::vector<Eigen::Vector3d>& vertices);

    static bounding_volume ritter_ctor(const std::vector<Eigen::Vector3d>& vertices);

    // getter
    inline bv_type         get_bv_type() const { return bv_type_; }
    inline Eigen::Vector3d get_center_point() const { return center_point_; }
    inline Eigen::Vector3d get_aabb_radius() const{ return radius_; }
    inline double          get_sphere_radius() const { return radius_.x(); }
    // setter
    void set_center_point(const Eigen::Vector3d& center_point) { center_point_ = center_point; }
    void set_aabb_radius(const Eigen::Vector3d radius) { radius_ = radius; }
    void set_sphere_radius(const double radius) { radius_.x() = radius; }

    bool intersection_test(const bounding_volume& other);
  private:
    bv_type bv_type_;
    Eigen::Vector3d center_point_;
    // if bv_type == SPHERE, only radius_.x() is valid.
    Eigen::Vector3d radius_;
};

// support functions
std::pair<int,int> most_separated_points_on_aabb(const std::vector<Eigen::Vector3d> &vertices);
bounding_volume sphere_from_distant_points(const std::vector<Eigen::Vector3d> &vertices);
void extend_sphere_to_point(bounding_volume& sphere, const Eigen::Vector3d& point);

}; // namespace hnll::physics