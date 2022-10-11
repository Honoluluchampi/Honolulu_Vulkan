// hnll
#include <geometry/perspective_frustum.hpp>
#include <utils/utils.hpp>

namespace hnll::geometry {

s_ptr<perspective_frustum> perspective_frustum::create(double fov_x, double fov_y, double near_z, double far_z)
{
 auto frustum = std::make_shared<perspective_frustum>();
 frustum->set_fov_x(fov_x);
 frustum->set_fov_y(fov_y);
 frustum->set_near_z(near_z);
 frustum->set_far_z(far_z);
 return frustum;
}

void perspective_frustum::update_planes(const utils::transform &tf)
{
  const auto& translation = tf.get_translation_ref();
  const auto  rotate_mat  = tf.rotate_mat3();

  // TODO : eigenize and delete
  vec3 tl = { translation.x, translation.y, translation.z };
  // update
  near_->point   = rotate_mat * vec3(0.f, 0.f, near_z_) + tl;
  far_->point    = rotate_mat * vec3(0.f, 0.f, far_z_)  + tl;
  left_->point   = tl;
  right_->point  = tl;
  top_->point    = tl;
  bottom_->point = tl;

  // default normal * rotate_mat
  near_->normal   = rotate_mat * vec3(0.f, 0.f, 1.f);
  far_->normal    = rotate_mat * vec3(0.f, 0.f, -1.f);
  left_->normal   = rotate_mat * vec3(std::cos(fov_x_), 0.f, std::sin(fov_x_));
  right_->normal  = rotate_mat * vec3(-std::cos(fov_x_), 0.f, std::sin(fov_x_));
  top_->normal    = rotate_mat * vec3(0.f, std::cos(fov_y_), std::sin(fov_y_));
  bottom_->normal = rotate_mat * vec3(0.f, -std::cos(fov_y_), std::sin(fov_y_));
}

} // namespace hnll::geometry