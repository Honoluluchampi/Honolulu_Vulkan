#pragma once

#include <hge_actor.hpp>
#include <hge_components/point_light_component.hpp>

// hve
#include <hve_frame_info.hpp>

// std
#include <unordered_map>

namespace hnll {

class HgePointLightManager : public HgeActor
{
  using map = std::unordered_map<HgeComponent::id_t, s_ptr<PointLightComponent>>;
  public:
    HgePointLightManager(GlobalUbo& ubo);

    // complete transport
    template <class SP> void addLightComp(SP&& spLightComp)
    { 
      HgeComponent::id_t id = spLightComp->getCompId();
      lightCompMap_.emplace(id, std::forward<SP>(spLightComp)); 
    }

    void updateActor(float dt) override;

  private:

    map lightCompMap_;
    GlobalUbo& ubo_;    
};

} // namespace hnll