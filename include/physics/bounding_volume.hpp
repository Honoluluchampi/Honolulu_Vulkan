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
    RITTOR,
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

    // getter
    bv_type get_bv_type() const { return bv_type_; }

    bool intersect_with(const bounding_volume& other){}
  private:
    bv_type bv_type_;
    Eigen::Vector3d center_point_;
    // if bv_type == SPHERE, only radius_.x() is valid.
    Eigen::Vector3d radius_;
};

}; // namespace hnll::physics