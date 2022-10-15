// hnll
#include <physics/engine.hpp>
#include <geometry/intersection.hpp>
#include <physics/collision_info.hpp>
#include <physics/collision_detector.hpp>

namespace hnll::physics {

// static members' declaration
u_ptr<collision_detector> engine::collision_detector_{};

} // namespace hnll::physics