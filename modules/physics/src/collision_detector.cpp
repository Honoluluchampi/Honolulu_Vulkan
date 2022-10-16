// hnll
#include <physics/collision_detector.hpp>
#include <game/components/rigid_component.hpp>
#include <geometry/bounding_volume.hpp>

namespace hnll::physics {

std::vector<s_ptr<game::rigid_component>> collision_detector::rigid_components_ = {};

} // namespace hnll::physics