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
  using map = std::unordered_map<HgeComponent::compId, s_ptr<PointLightComponent>>;
  public:
    HgePointLightManager(GlobalUbo& ubo);

    // complete transport
    template <class SP> void addLightComp(SP&& spLightComp)
    { 
      HgeComponent::compId id = spLightComp->getCompId();
      lightCompMap_.emplace(id, std::forward<SP>(spLightComp)); 
    }
    void removeLightComp(HgeComponent::compId id)
    { lightCompMap_.erase(id); }

    void updateActor(float dt) override;

  private:

    map lightCompMap_;
    GlobalUbo& ubo_;    
};

} // namespace hnll