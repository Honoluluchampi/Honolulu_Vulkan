#pragma once

#include <memory>

template <typename T> using u_ptr = std::unique_ptr<T>;
template <typename T> using s_ptr = std::shared_ptr<T>;

namespace hnll {

// forward declaration
namespace geometry { class collision_detector; }

namespace physics {

class engine
{
  public:
    engine(){}
    ~engine(){}

  private:
    static u_ptr<geometry::collision_detector> collision_detector_up_;
};

}
} // namespace hnll::physics