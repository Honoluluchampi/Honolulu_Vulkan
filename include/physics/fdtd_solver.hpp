#pragma once

#include <vector>
#include <memory>

namespace hnll {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

namespace physics {

class fdtd_solver
{
  public:
    fdtd_solver() = default;

  private:
    // stable restriction dx_ / dt_ > sqrt(stiffness_ / density_)
    double dx_;
    double dt_;
    double stiffness_;
    double density_;
    std::vector<double> input_;
    std::vector<double> stress_;
    std::vector<double> particle_velocity_;
};

}} // namespace hnll::physics