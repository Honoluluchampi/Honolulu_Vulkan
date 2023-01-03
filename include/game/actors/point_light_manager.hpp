#pragma once

// hnll
#include <game/actor.hpp>
#include <game/components/point_light_component.hpp>
#include <utils/rendering_utils.hpp>

// std
#include <unordered_map>

namespace hnll {

namespace game {

using point_light_map = std::unordered_map<component_id, s_ptr<point_light_component>>;

class point_light_manager : public actor
{
  public:
    point_light_manager(utils::global_ubo& ubo);
    ~point_light_manager(){}

    // complete transport
    template <class SP> void add_light_comp(SP&& light_comp_sp)
    { component_id id = light_comp_sp->get_id(); light_comp_map_.emplace(id, std::forward<SP>(light_comp_sp)); }
    void remove_light_comp(component_id id) { light_comp_map_.erase(id); }

    void update_actor(float dt) override;

  private:
    point_light_map light_comp_map_;
    utils::global_ubo& ubo_;
};

} // namespace game
} // namespace hnll