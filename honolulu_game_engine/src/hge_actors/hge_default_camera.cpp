#include <hge_actors/hge_default_camera.hpp>
#include <hge_game.hpp>
#include <hve_app.hpp>

namespace hnll {

HgeCamera::HgeCamera(Hve& hve) : HgeActor()
{
  spViewerComp_m = std::make_shared<ViewerComponent>(transform_m, hve.hveRenderer());
  // set initial position
  transform_m.translation_m.z = -2.5f;
  
  auto keyComp = std::make_shared<KeyboardMovementComponent>(hve.passGLFWwindow(), transform_m);
  
  // key move must be updated before view
  addSharedComponent(keyComp);
  // shared by hveApp
  addSharedComponent(spViewerComp_m);
}

} // namesapce hnll