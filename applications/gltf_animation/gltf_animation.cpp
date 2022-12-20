// hnll
#include <game/engine.hpp>
#include <game/shading_systems/mesh_model_shading_system.hpp>
#include <game/shading_systems/grid_shading_system.hpp>
#include <game/shading_systems/skinning_mesh_model_shading_system.hpp>
#include <game/components/mesh_component.hpp>
#include <game/components/skinning_mesh_component.hpp>
#include <game/components/point_light_component.hpp>

// std
#include <iostream>

namespace hnll {

class gltf_animation : public game::engine
{
  public:
    gltf_animation() : game::engine("gltf animation")
    {
      setup_shading_systems();

//      setup_lights();

      add_model();
    }

  private:
    void setup_shading_systems()
    {
      auto mesh_model_shader = game::mesh_model_shading_system::create(get_graphics_device());
      add_shading_system(std::move(mesh_model_shader));
      auto grid_shader = game::grid_shading_system::create(get_graphics_device());
      add_shading_system(std::move(grid_shader));
      auto skinning_shader = game::skinning_mesh_model_shading_system::create(get_graphics_device());
      add_shading_system(std::move(skinning_shader));
    }

    void setup_lights()
    {
      float light_intensity = 20.f;
      std::vector<glm::vec3> positions;
      float position_radius = 8.f;
      for (int i = 0; i < 6; i++) {
        positions.push_back({position_radius * std::sin(M_PI/3.f * i), -2.f, position_radius * std::cos(M_PI/3.f * i)});
      }
      positions.push_back({0.f, position_radius, 0.f});
      positions.push_back({0.f, -position_radius, 0.f});

      for (const auto& position : positions) {
        auto light = hnll::game::actor::create();
        auto light_component = hnll::game::point_light_component::create(light, light_intensity, 0.f);
        add_point_light(light, light_component);
        light->set_translation(position);
      }
    }

    void add_model()
    {
      auto actor = game::actor::create();
      auto obj = game::skinning_mesh_component::create(actor, "human.glb");
      actor->set_scale({0.3f, 0.3f, 0.3f});
      actor->set_rotation({M_PI, 0.f, 0.f});
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