#include <hge_actors/hge_default_camera.hpp>
#include <hge_game.hpp>

namespace hnll {

s_ptr<HgeCamera> HgeCamera::create(HgeGame* owner)
{
  return s_ptr<HgeCamera>(new HgeCamera(owner));
}

HgeCamera::HgeCamera(HgeGame* owner) : HgeActor(owner)
{
  auto& hve = owner->hve();
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