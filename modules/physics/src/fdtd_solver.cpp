#include <physics/fdtd_solver.hpp>

namespace hnll::physics {

double VELOCITY_RATIO = 0.95;

void fdtd_solver::solve(std::vector<double> _input, double _duration)
{

}

fdtd_solver* fdtd_solver::set_max_stable_dx()
{
  if (phase_velocity_ > 0.f) {
    dx_ = VELOCITY_RATIO * phase_velocity_ * dt_;
  }
  return this;
}

fdtd_solver* fdtd_solver::set_min_stable_dt()
{
  if (phase_velocity_ > 0.f) {
    dt_ = dx_ / VELOCITY_RATIO / phase_velocity_;
  }
  return this;
}
} // namespace hnll::physics