// hnll
#include <game/engine.hpp>
#include <geometry/intersection.hpp>
#include <physics/engine.hpp>
#include <physics/collision_info.hpp>
#include <physics/collision_detector.hpp>

namespace hnll::physics {

// static members' declaration
u_ptr<collision_detector> engine::collision_detector_{};


void engine::re_update()
{
  adjust_intersection();
}

void engine::adjust_intersection()
{
  // actors will be re-updated in this function
  auto collision_info_list = physics::collision_detector::intersection_test();
  for (const auto& info : collision_info_list) {
    game::engine::get_active_actor(info.actor_a).re_update(info);
    game::engine::get_active_actor(info.actor_b).re_update(info);
  }
}
} // namespace hnll::physics