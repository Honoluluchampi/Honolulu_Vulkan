#pragma once

namespace hnll {
namespace physics {

class particle_component
{
  public:
    particle_component();
    ~particle_component();

  private:
    double position_[3];
    double velocity_[3];
};

} // namespace physics
} // namespace hnll