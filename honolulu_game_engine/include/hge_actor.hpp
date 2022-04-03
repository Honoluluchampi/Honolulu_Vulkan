#pragma once

#include <hge_component.hpp>

// std
#include <vector>
#include <memory>
#include <unordered_map>

namespace hnll {

class HgeActor
{
  public:
    using id_t = unsigned int;
    using map = std::unordered_map<id_t, std::unique_ptr<HgeActor>>;

    // hgeActor can be created only by this fuction
    static HgeActor createActor()
    { static id_t currentId = 0; return HgeActor{currentId++}; }

    enum class state
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
    virtual ~HgeActor(){}

    virtual void update(float dt){}

    // takes std::unique_ptr<HgeComponent>
    template <class U>
    void addUniqueComponent(U&& comp)
    { uniqueComponents_m.push_back(std::move(comp)); }
    // void addUniqueComponent(std::unique_ptr<HgeComponent>&& comp)
    // { uniqueComponents_m.push_back(std::move(comp)); }

    // takes std::shared_ptr<HgeComponent>
    template <class S>
    void addSharedComponent(S&& comp)
    { sharedComponents_m.push_back(std::forward<S>(comp)); }
    // void addSharedComponent(std::)

    inline id_t getId() const { return id_m; }
    inline const state& getActorState() const { return state_m; }
  private:
    HgeActor(id_t id) : id_m(id) {}

    id_t id_m;
    state state_m = state::ACTIVE;
    // would be shared?
    std::vector<std::unique_ptr<HgeComponent>> uniqueComponents_m;
    std::vector<std::shared_ptr<HgeComponent>> sharedComponents_m; 
};

} // namespace hnll