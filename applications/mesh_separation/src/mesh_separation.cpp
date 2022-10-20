// hnll
#include <game/engine.hpp>
#include <game/actors/default_camera.hpp>
#include <game/components/point_light_component.hpp>
#include <game/components/mesh_component.hpp>
#include <geometry/mesh_model.hpp>
#include <geometry/half_edge.hpp>
#include <geometry/mesh_separation.hpp>

using namespace hnll;

using vec3 = Eigen::Vector3d;

class app : public game::engine
{
  public:
   auto add_separated_object(const std::string& filename, geometry::mesh_separation::criterion crtr)
    {
      std::vector<s_ptr<game::actor>> mesh_lets;
      auto sphere_geometry = geometry::mesh_model::create_from_obj_file(filename);
      auto sphere_meshlets = geometry::mesh_separation::separate(sphere_geometry, crtr);
      for (auto& ml : sphere_meshlets) {
        auto ml_actor = game::actor::create();
        auto ml_graphics = graphics::mesh_model::create_from_geometry_mesh_model(get_graphics_device(), ml);
        auto ml_mesh_comp = game::mesh_component::create(ml_actor, std::move(ml_graphics));
        game::engine::add_actor(ml_actor);
        ml_actor->set_rotation({M_PI, 0.f, 0.f});
        mesh_lets.push_back(ml_actor);
      }
      return mesh_lets;
    }

    app() : game::engine("mesh_separation")
    {
      // set camera position
      camera_up_->set_translation(glm::vec3{0.f, -2.f, -7.f});
      // add light
      setup_lights();
      auto mv = add_separated_object("bunny.obj", hnll::geometry::mesh_separation::criterion::MINIMIZE_BOUNDING_SPHERE);
//      auto bs = add_separated_object("bunny.obj", hnll::geometry::mesh_separation::criterion::MINIMIZE_BOUNDING_SPHERE);

      for (auto& m : mv) {
        m->set_rotation({M_PI, -0.25f, 0.f});
      }
//      for (auto& b : bs) {
//        b->set_translation({-2.f, 0.f, 0.f});
//      }
    }

    ~app(){}

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
    };
};

int main() {
  app app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}