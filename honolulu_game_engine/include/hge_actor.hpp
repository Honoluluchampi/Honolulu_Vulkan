#pragma once

#include <hge_component.hpp>
#include <hge_components/hge_renderable_component.hpp>

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

    // takes std::unique_ptr<HgeComponent>
    // template <class U>
    // void addUniqueComponent(U&& comp)
    // { uniqueComponents_m.push_back(std::move(comp)); }
    void addComponent(u_ptr<HgeComponent>&& comp)
    { uniqueComponents_.emplace_back(std::move(comp)); }

    // takes std::shared_ptr<HgeComponent>
    // template <class S>
    // void addSharedComponent(S&& comp)
    // { sharedComponents_m.push_back(std::forward<S>(comp)); }
    void addComponent(const s_ptr<HgeComponent>& comp)
    { sharedComponents_.emplace_back(comp); }
    void addComponent(s_ptr<HgeComponent>&& comp)
    { sharedComponents_.emplace_back(std::move(comp)); }

     // TODO : overload addSharedComponent
    // takes std::shared_ptr<RenderableComponent>
    template <class S>
    void addRenderableComponent(S&& comp)
    { 
      renderableComponent_ = std::forward<S>(comp); 
    }

    // TODO : choose rcomp and replace in multiple rcomps
    // template <class S>
    // void replaceRenderableComponent(S&& comp)
    // {
    //   renderableComponent_ = std::forward<S>(comp);
    // }

    // TODO : not to use dynamic_cast
    inline s_ptr<HgeRenderableComponent> getRenderableComponent()
    { return renderableComponent_; }

    inline actorId getId() const { return id_m; }
    inline const state& getActorState() const { return state_m; }

    inline bool isRenderable() const { return renderableComponent_ != nullptr; }

  private:
    actorId id_m;
    state state_m = state::ACTIVE;

    // would be shared?
    std::vector<u_ptr<HgeComponent>> uniqueComponents_;
    std::vector<s_ptr<HgeComponent>> sharedComponents_;
    // TODO : multiple renderableComponent for one actor 
    s_ptr<HgeRenderableComponent> renderableComponent_ = nullptr;
};

} // namespace hnll