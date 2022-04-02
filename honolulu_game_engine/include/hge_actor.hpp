#pragma once

#include <hge_component.hpp>

// std
#include <vector>
#include <memory>

namespace hnll {

class HgeActor
{
  public:
    enum class ActorState
    {
       ACTIVE,
       PAUSED,
       DEAD
    };

    HgeActor();
    virtual ~HgeActor();

    virtual void update(float dt);
  
  private:
    // would be shared?
    std::vector<std::unique_ptr<HgeComponent>> upComponents_m; 
};

} // namespace hnll