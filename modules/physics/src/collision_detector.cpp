// hnll
#include <physics/collision_detector.hpp>
#include <physics/collision_info.hpp>
#include <game/components/rigid_component.hpp>
#include <geometry/bounding_volume.hpp>
#include <geometry/intersection.hpp>

namespace hnll::physics {

std::vector<s_ptr<game::rigid_component>> collision_detector::rigid_components_ = {};

std::vector<collision_info> collision_detector::intersection_test()
{
  std::vector<collision_info> res;

  int rc_count = rigid_components_.size();

  // full search
  for (int i = 0; i < rc_count - 1; i++) {
    for (int j = i; j < rc_count; j++) {
      auto& a = rigid_components_[i];
      auto& b = rigid_components_[j];
      // skip if the owners are the same
      if (a->get_id() == b->get_id()) continue;
      if (auto depth = geometry::intersection::test_bounding_volumes(a->get_bounding_volume(), b->get_bounding_volume()); depth) {
        // create collision_info
        collision_info info;
        info.intersection_depth = depth;
        info.actor_a = a->get_owner_id();
        info.actor_b = b->get_owner_id();
        res.emplace_back(std::move(info));
      }
    }
  }

  return res;
}

} // namespace hnll::physics