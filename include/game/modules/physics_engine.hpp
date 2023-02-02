#pragma once

// hnll
#include <utils/common_using.hpp>

namespace hnll {

// forward declaration
namespace physics { class collision_detector; }

namespace game {

class physics_engine {
  public:
    physics_engine() = default;
    ~physics_engine() = default;

    void re_update();

    void adjust_intersection();

  private:
    static u_ptr<physics::collision_detector> collision_detector_;
};

}} // namespace hnll::game