#include <game/engine.hpp>
#include <game/components/mesh_component.hpp>

class falling_ball_app : public hnll::game::engine
{
  public:
    falling_ball_app() : hnll::game::engine("falling ball")
    {
      auto ball = hnll::game::actor::create();
      auto ball_mesh = hnll::game::engine::get_mesh_model_sp("sphere");
      auto ball_mesh_vertices_list = ball_mesh->get_vertices_list();
      auto ball_mesh_comp = hnll::game::mesh_component::create(ball, std::move(ball_mesh));
      std::cout << ball_mesh_vertices_list[0].position.x() << std::endl;
      hnll::game::engine::add_actor(ball);
    }
};

int main()
{
  falling_ball_app app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}