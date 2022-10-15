#include <physics/collision_detector.hpp>

namespace hnll::physics {

std::vector<s_ptr<rigid_component>> collision_detector::rigid_components_ = {};

} // namespace hnll::physics