#pragma once

// std
#include <vector>

namespace hnll::physics {

// forward declaration
class bounding_volume;

class collision_detector
{
  public:
    collision_detector();

  private:
    std::vector<bounding_volume> rigid_objects_;
};

} // namespace hnll::physics