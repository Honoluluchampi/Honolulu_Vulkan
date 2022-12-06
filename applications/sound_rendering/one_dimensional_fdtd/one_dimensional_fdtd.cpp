// hnll
#include <game/engine.hpp>
#include <physics/engine.hpp>
#include <physics/fdtd_solver.hpp>

namespace hnll {

class app : public game::engine
{
  public:
    app() : game::engine("one dimensional fdtd")
    {
      init_solver();
      create_input();
      solver_->solve(input_, 5.0f);
    }

    ~app() = default;

  private:
    void init_solver()
    {
      solver_ = physics::fdtd_solver::create();
      solver_->set_dt(0.1)
             ->set_phase_velocity(3.f)
             ->set_max_stable_dx();
    }

    void create_input()
    {
      double dt        = solver_->get_dt();
      double duration  = 3.f;
      double frequency = 1.f;
      double amplitude = 2.f;

      input_.resize(duration / dt);

      for (int i = 0; i < input_.size(); i++) {
        input_[i] = amplitude * std::sin(frequency * dt * 2.0 * M_PI * i);
      }
    }

    std::vector<double> input_;
    u_ptr<physics::fdtd_solver> solver_;
};

} // namespace hnll

int main()
{
  hnll::app app{};

  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}