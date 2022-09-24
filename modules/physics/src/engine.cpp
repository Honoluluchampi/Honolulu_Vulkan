// hnll
#include <physics/engine.hpp>
#include <physics/collision_detector.hpp>

namespace hnll::physics {

// static members' declaration
u_ptr<collision_detector> engine::collision_detector_up_{};

} // namespace hnll::physics