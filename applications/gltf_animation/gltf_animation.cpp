// hnll
#include <game/engine.hpp>
#include <game/shading_systems/mesh_model_shading_system.hpp>

// std
#include <iostream>

namespace hnll {

class gltf_animation : public game::engine
{
  public:
    gltf_animation() : game::engine("gltf animation")
    {
      setup_shading_systems();
    }

  private:
    void setup_shading_systems()
    {
      auto mesh_model_shader = game::mesh_model_shading_system::create(get_graphics_device());
      add_shading_system(std::move(mesh_model_shader));
    }
};
} // namespace hnll

int main()
{
  hnll::gltf_animation app;

  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}