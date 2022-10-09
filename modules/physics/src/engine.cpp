// hnll
#include <physics/engine.hpp>
#include <geometry/collision_detector.hpp>

namespace hnll::physics {

// static members' declaration
u_ptr<geometry::collision_detector> engine::collision_detector_up_{};

} // namespace hnll::physics