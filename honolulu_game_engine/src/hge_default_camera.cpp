#include <hge_default_camera.hpp>

namespace hnll {

HgeCamera::HgeCamera(GLFWwindow* window, HveRenderer& renderer) : HgeActor()
{
  spViewerComp_m = std::make_shared<ViewerComponent>(transform_m, renderer);
  // set initial position
  transform_m.translation_m.z = -2.5f;
  
  auto keyComp = std::make_unique<KeyboardMovementComponent>(window, transform_m);
  
  // key move must be updated before view
  addUniqueComponent(keyComp);
  // shared by hveApp
  addSharedComponent(spViewerComp_m);
}

} // namesapce hnll