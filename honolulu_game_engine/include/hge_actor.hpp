#pragma once

#include <hge_component.hpp>

// std
#include <vector>
#include <memory>

namespace hnll {

class HgeActor
{
  public:
    using id_t = unsigned int;
    // hgeActor can be created only by this fuction
    static HgeActor createActor()
    { static id_t currentId = 0; return HgeActor{currentId++}; }

    enum class ActorState
    {
       ACTIVE,
       PAUSED,
       DEAD
    };

    // uncopyable, movable
    HgeActor(const HgeActor &) = delete;
    HgeActor& operator=(const HgeActor &) = delete;
    HgeActor(HgeActor &&) = default;
    HgeActor& operator=(HgeActor &&) = default;
    virtual ~HgeActor();

    virtual void update(float dt);
  
  private:
    explicit HgeActor(id_t id);

    id_t id_m;
    // would be shared?
    std::vector<std::unique_ptr<HgeComponent>> upComponents_m; 
};

} // namespace hnll