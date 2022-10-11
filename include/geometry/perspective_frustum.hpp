#pragma once

// std
#include <memory>

// lib
#include <eigen3/Eigen/Dense>

namespace hnll {

template<typename T> using s_ptr = std::shared_ptr<T>;
template<typename T> using u_ptr = std::unique_ptr<T>;
using vec3 = Eigen::Vector3d;
// forward declaration
namespace utils { class transform; }

namespace geometry {

struct plane
{
  vec3 point;
  vec3 normal;
  // plane's normal is guaranteed to be normalized
  plane(const vec3& point_, const vec3& normal_) : point(point_), normal(normal_) { normal.normalize(); }
  plane() : normal({0.f, -1.f, 0.f}) {}
};

class perspective_frustum
{
  public:
    enum class update_fov_x { ON, OFF };

    static s_ptr<perspective_frustum> create(double fov_x, double fov_y, double near_z_, double far_z_);

    void update_planes(const utils::transform& tf);

    // getter
    vec3 get_near_n()   const { return near_->normal; }
    vec3 get_far_n()    const { return far_->normal; }
    vec3 get_left_n()   const { return left_->normal; }
    vec3 get_right_n()  const { return right_->normal; }
    vec3 get_top_n()    const { return top_->normal; }
    vec3 get_bottom_n() const { return bottom_->normal; }
    vec3 get_near_p()   const { return near_->point;}
    vec3 get_far_p()    const { return far_->point;}
    vec3 get_left_p()   const { return left_->point;}
    vec3 get_right_p()  const { return right_->point;}
    vec3 get_top_p()    const { return top_->point;}
    vec3 get_bottom_p() const { return bottom_->point;}
    const plane& get_near_ref()   const { return *near_; }
    const plane& get_far_ref()    const { return *far_; }
    const plane& get_left_ref()   const { return *left_; }
    const plane& get_right_ref()  const { return *right_; }
    const plane& get_top_ref()    const { return *top_; }
    const plane& get_bottom_ref() const { return *bottom_; }
    double get_near_z() const { return near_z_; }
    double get_far_z()  const { return far_z_; }

    // setter
    void set_fov_x(double fx)  { fov_x_ = fx; }
    void set_fov_y(double fy)  { fov_y_ = fy; }
    void set_near_z(double nz) { near_z_ = nz; }
    void set_far_z(double fz)  { far_z_ = fz; }

  private:
    u_ptr<plane>  near_, far_, left_, right_, top_, bottom_;
    double fov_x_ = M_PI / 4.f, fov_y_ = M_PI / 4.f, near_z_, far_z_;
    update_fov_x update_fov_x_ = update_fov_x::ON;
};


}} // namespace hnll::geometry
