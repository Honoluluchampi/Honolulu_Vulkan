// hnll
#include <game/actors/default_camera.hpp>
#include <graphics/engine.hpp>

namespace hnll {

default_camera::default_camera(engine& hve) : actor()
{
  viewer_comp_sp_ = std::make_shared<viewer_component>(transform_, hve.hveRenderer());
  // set initial position
  transform_.translation.z = -2.5f;
  
  auto keyComp = std::make_shared<keyboard_movement_component>(hve.passGLFWwindow(), transform_);
  
  // key move must be updated before view
  add_component(keyComp);
  // shared by hveApp
  add_component(viewer_comp_sp_);
}

} // namesapce hnll