#pragma once

// hnll
#include <game/actor.hpp>
#include <game/components/point_light_component.hpp>

// std
#include <unordered_map>

namespace hnll {

// forward declaration
namespace graphics { class global_ubo; }

namespace game {
class point_light_manager : public actor
{
  using map = std::unordered_map<component::id, s_ptr<point_light_component>>;
  public:
    point_light_manager(hnll::graphics::global_ubo& ubo);
    ~point_light_manager(){}

    // complete transport
    template <class SP> void add_light_comp(SP&& light_comp_sp)
    { component::id id = light_comp_sp->get_id(); light_comp_map_.emplace(id, std::forward<SP>(light_comp_sp)); }
    void remove_light_comp(component::id id) { light_comp_map_.erase(id); }

    void update_actor(float dt) override;

  private:
    map light_comp_map_;
    hnll::graphics::global_ubo& ubo_;    
};

} // namespace game
} // namespace hnll