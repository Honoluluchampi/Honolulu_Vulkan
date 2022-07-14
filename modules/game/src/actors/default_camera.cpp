// hnll
#include <game/actors/default_camera.hpp>
#include <graphics/engine.hpp>

namespace hnll {

HgeCamera::HgeCamera(Hve& hve) : HgeActor()
{
  spViewerComp_m = std::make_shared<ViewerComponent>(transform_m, hve.hveRenderer());
  // set initial position
  transform_m.translation_m.z = -2.5f;
  
  auto keyComp = std::make_shared<KeyboardMovementComponent>(hve.passGLFWwindow(), transform_m);
  
  // key move must be updated before view
  addComponent(keyComp);
  // shared by hveApp
  addComponent(spViewerComp_m);
}

} // namesapce hnll