#include <hge_default_camera.hpp>

namespace hnll {

HgeCamera::HgeCamera(GLFWwindow* window, HveRenderer& renderer) : HgeActor()
{
  spViewerComp_m = std::make_unique<ViewerComponent>(*upTransform_m, renderer);
  // set initial position
  upTransform_m = std::make_unique<Transform>();
  upTransform_m->translation_m.z = -2.5f;
  
  auto keyComp = std::make_unique<KeyboardMovementComponent>(window, *upTransform_m);
  
  // key move must be updated before view
  addUniqueComponent(keyComp);
  // shared by hveApp
  addSharedComponent(spViewerComp_m);
}

} // namesapce hnll