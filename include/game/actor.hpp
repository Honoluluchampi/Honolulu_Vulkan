#pragma once

// hnll
#include <game/engine.hpp>
#include <game/component.hpp>
#include <game/components/renderable_component.hpp>
#include <utils/utils.hpp>

// std
#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>

template <class T> using s_ptr = std::shared_ptr<T>;
template <class T> using u_ptr = std::unique_ptr<T>;

namespace hnll {
namespace game {

// forward declaration
//class engine;

class actor
{
  public:
    enum class state { ACTIVE, PAUSED, DEAD };

    // hgeActor can be created only by this fuction
    actor();
    // uncopyable, movable
    actor(const actor &) = delete;
    actor& operator=(const actor &) = delete;
    actor(actor &&) = default;
    actor& operator=(actor &&) = default;
    virtual ~actor(){}
    template <class actor_class = actor, class... args>
    static s_ptr<actor_class> create(args... ags)
    {
      auto actor = std::make_shared<actor_class>(ags...);
      // create s_ptr of actor perform as actor
      std::shared_ptr<hnll::game::actor> prt_for_actor_map = actor;
      // register it to the actor map
      hnll::game::engine::add_actor(actor);
      return actor;
    }

    void update(float dt);
    void update_components(float dt);
    virtual void update_actor(float dt) {}

#ifndef IMGUI_DISABLED
    void update_gui();
    void update_component_imgui();
    virtual void update_actor_imgui(){}
#endif

    // getter
    inline id get_id() const { return id_; }
    inline const state get_actor_state() const { return state_; }
    inline s_ptr<renderable_component> get_renderable_component() { return renderable_component_; }
    inline bool is_renderable() const { return renderable_component_ != nullptr; }
    s_ptr<hnll::utils::transform> get_transform_sp();
    // setter
    void add_component(u_ptr<component>&& comp) { unique_components_.emplace_back(std::move(comp)); }
    void add_component(s_ptr<component>&& comp) { shared_components_.emplace_back(std::move(comp)); }
    void add_component(const s_ptr<component>& comp) { shared_components_.emplace_back(comp); }
    // takes std::shared_ptr<RenderableComponent>
    void set_renderable_component(s_ptr<renderable_component>&& comp);
    void set_renderable_component(const s_ptr<renderable_component>& comp);
    inline void set_actor_state(state st) { state_ = st; }


  private:
    actor_id id_;
    state state_ = state::ACTIVE;

    std::vector<u_ptr<component>> unique_components_;
    std::vector<s_ptr<component>> shared_components_;
    // TODO : multiple renderableComponent for one actor 
    s_ptr<renderable_component> renderable_component_ = nullptr;
    s_ptr<hnll::utils::transform> transform_sp_;
};

} // namespace game
} // namespace hnll