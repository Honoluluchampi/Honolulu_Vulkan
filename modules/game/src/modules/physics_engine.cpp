// hnll
#include <game/engine.hpp>
#include <game/modules/physics_engine.hpp>
#include <geometry/intersection.hpp>
#include <physics/collision_info.hpp>
#include <physics/collision_detector.hpp>

namespace hnll::game {

// static members' declaration
u_ptr<physics::collision_detector> physics_engine::collision_detector_{};


void physics_engine::re_update()
{
  adjust_intersection();
}

void physics_engine::adjust_intersection()
{
  // actors will be re-updated in this function
  auto collision_info_list = physics::collision_detector::intersection_test();
  for (const auto& info : collision_info_list) {
    game::engine::get_active_actor(info.actor_a).re_update(info);
    game::engine::get_active_actor(info.actor_b).re_update(info);
  }
}
} // namespace hnll::physics