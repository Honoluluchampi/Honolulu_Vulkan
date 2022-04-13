#pragma once 

#include <hge_actor.hpp>
#include <hge_components/viewer_component.hpp>
#include <hge_components/keyboardMovementComponent.hpp>
#include <utility.hpp>

namespace hnll {

class HgeCamera : public HgeActor
{
public:
  HgeCamera(GLFWwindow* window, HveRenderer& renderer);
  ~HgeCamera(){}

  HgeCamera(const HgeCamera &) = delete;
  HgeCamera& operator=(const HgeCamera &) = delete;
  HgeCamera(HgeCamera &&) = default;
  HgeCamera& operator=(HgeCamera &&) = default;

  inline Transform& getTransform() { return transform_m; }
  template<class V> void setTranslation(V&& vec) 
  { transform_m.translation_m = std::forward<V>(vec); }    
  template<class V> void setScale(V&& vec) 
  { transform_m.scale_m = std::forward<V>(vec); }
  template<class V> void setRotation(V&& vec) 
  { transform_m.rotation_m = std::forward<V>(vec); }  

  inline s_ptr<ViewerComponent> viewerComponent() const { return spViewerComp_m; }  

private:
  Transform transform_m {};
  s_ptr<ViewerComponent> spViewerComp_m;
};

} // namespace hnll