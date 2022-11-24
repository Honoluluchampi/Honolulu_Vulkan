#pragma once

#include <vector>
#include <memory>

namespace hnll {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

namespace physics {

// temporally one-dimensional implementation
class fdtd_solver
{
  public:
    static u_ptr<fdtd_solver> create() { return std::make_unique<fdtd_solver>(); }

    fdtd_solver() = default;

    // _duration is in seconds
    void solve(std::vector<double> _input, double _duration);

    // setter
    double get_dx()             const { return dx_; }
    double get_dt()             const { return dt_; }
    double get_stiffness()      const { return stiffness_; }
    double get_density_()       const { return density_; }
    double get_phase_velocity() const { return phase_velocity_; }

    // setter
    fdtd_solver* set_dx(double _dx)             { dx_ = _dx; return this; }
    fdtd_solver* set_dt(double _dt)             { dt_ = _dt; return this; }
    fdtd_solver* set_stiffness(double _sn)      { stiffness_ = _sn;      return this; }
    fdtd_solver* set_density_(double _ds)       { density_ = _ds;        return this; }
    fdtd_solver* set_phase_velocity(double _pv) { phase_velocity_ = _pv; return this; }
    fdtd_solver* set_max_stable_dx();
    fdtd_solver* set_min_stable_dt();

  private:
    // stable restriction dx_ / dt_ > sqrt(stiffness_ / density_)
    double dx_;
    double dt_;
    double stiffness_;
    double density_;
    double phase_velocity_ = -1.f;
    std::vector<double> stress_;
    std::vector<double> particle_velocity_;
};

}} // namespace hnll::physics