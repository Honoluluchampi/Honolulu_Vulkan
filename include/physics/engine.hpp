#pragma once

#include <memory>

namespace hnll {

template <typename T> using u_ptr = std::unique_ptr<T>;
template <typename T> using s_ptr = std::shared_ptr<T>;

namespace physics {

// forward declaration
class collision_detector;

class engine
{
  public:
    engine()  = default;
    ~engine() = default;

    void re_update();
    void adjust_intersection();

  private:
    static u_ptr<collision_detector> collision_detector_;
};

}
} // namespace hnll::physics