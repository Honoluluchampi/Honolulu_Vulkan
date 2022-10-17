// hnll
#include <game/actors/default_camera.hpp>
#include <graphics/engine.hpp>

namespace hnll::game {

default_camera::default_camera(hnll::graphics::engine& hve) : actor()
{
  viewer_comp_sp_ = std::make_shared<viewer_component>(transform_, hve.get_renderer());
  // set initial position
  transform_.translation.z = -7.f;
  transform_.translation.y = -2.f;
  
  key_comp_sp_ = std::make_shared<keyboard_movement_component>(hve.get_glfw_window(), transform_);
  
  // key move must be updated before view
  add_component(key_comp_sp_);
  // shared by hveApp
  add_component(viewer_comp_sp_);
}

} // namespace hnll::game