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

  auto rc_count = rigid_components_.size();

  // full search
  for (int i = 0; i < rc_count - 1; i++) {
    for (int j = i; j < rc_count; j++) {
      auto& a = rigid_components_[i];
      auto& b = rigid_components_[j];
      if (geometry::intersection::test_bounding_volumes(a->get_bounding_volume(), b->get_bounding_volume())) {
        // create collision_info
        collision_info info;
        info.actor_a_ = a->get_owner_id();
        info.actor_b_ = b->get_owner_id();
        res.emplace_back(std::move(info));
      }
    }
  }

  return res;
}

} // namespace hnll::physics