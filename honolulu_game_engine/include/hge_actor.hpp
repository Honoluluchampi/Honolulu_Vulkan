#pragma once

#include <hge_component.hpp>

// std
#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>

template <class T> using s_ptr = std::shared_ptr<T>;
template <class T> using u_ptr = std::unique_ptr<T>;

namespace hnll {

// forward declaration
class HgeGame;

class HgeActor
{
  public:
    using actorId = unsigned int;
    using map = std::unordered_map<actorId, s_ptr<HgeActor>>;

    // hgeActor can be created only by this fuction
    HgeActor();
    
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

    void update(float dt);
    virtual void updateActor(float dt) {}
    void updateComponents(float dt);

#ifndef __IMGUI_DISABLED
    void updateImgui();
    virtual void updateActorImgui(){}
    void updateComponentsImgui();
#endif

    // TODO : override addComp func to add specific comp
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

     // TODO : overload addSharedComponent
    // takes std::shared_ptr<RenderableComponent>
    template <class S>
    void addRenderableComponent(S&& comp)
    { 
      isRenderable_m = true; 
      renderableComponentID_m = sharedComponents_m.size();
      sharedComponents_m.push_back(std::forward<S>(comp)); 
    }

    template <class S>
    void replaceRenderableComponent(S&& comp)
    {
      if (isRenderable_m)
        sharedComponents_m[renderableComponentID_m] = std::forward<S>(comp);
      else std::cout << "add RenderableComponent first!" << std::endl;
    }

    HgeComponent& getRenderableComponent()
    {
      return *sharedComponents_m[renderableComponentID_m];
    }

    inline actorId getId() const { return id_m; }
    inline const state& getActorState() const { return state_m; }

    inline bool isRenderable() const { return isRenderable_m; }

  private:
    actorId id_m;
    state state_m = state::ACTIVE;

    // would be shared?
    std::vector<std::unique_ptr<HgeComponent>> uniqueComponents_m;
    std::vector<std::shared_ptr<HgeComponent>> sharedComponents_m; 
    bool isRenderable_m = false;
    actorId renderableComponentID_m = -1;
};

} // namespace hnll