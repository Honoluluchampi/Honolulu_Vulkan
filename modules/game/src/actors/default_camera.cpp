// hnll
#include <game/actors/default_camera.hpp>
#include <game/modules/graphics_engine.hpp>
#include <geometry/perspective_frustum.hpp>

namespace hnll::game {

default_camera::default_camera(graphics_engine& hve) : actor()
{
  viewer_comp_sp_ = std::make_shared<viewer_component>(transform_, hve.get_renderer_r());
  // set initial position
  transform_.translation.z = -7.f;
  transform_.translation.y = -2.f;
  
  key_comp_sp_ = std::make_shared<keyboard_movement_component>(hve.get_glfw_window(), transform_);
  
  // key move must be updated before view
  add_component(key_comp_sp_);
  // shared by hveApp
  add_component(viewer_comp_sp_);
}

utils::frustum_info default_camera::get_frustum_info() const
{
  auto& frustum = viewer_comp_sp_->get_perspective_frustum_ref();
  return {
    { transform_.translation.x, transform_.translation.y, transform_.translation.z},
    frustum.get_near_p().cast<float>(),
    frustum.get_far_p().cast<float>(),
    frustum.get_top_n().cast<float>(),
    frustum.get_bottom_n().cast<float>(),
    frustum.get_right_n().cast<float>(),
    frustum.get_left_n().cast<float>(),
    frustum.get_near_n().cast<float>(),
    frustum.get_far_n().cast<float>(),
  };
}

} // namespace hnll::game