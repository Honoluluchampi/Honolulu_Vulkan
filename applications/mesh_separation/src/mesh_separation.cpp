// hnll
#include <game/engine.hpp>
#include <game/actors/default_camera.hpp>
#include <game/components/point_light_component.hpp>
#include <game/components/mesh_component.hpp>
#include <geometry/mesh_model.hpp>
#include <geometry/half_edge.hpp>

using namespace hnll;

using vec3 = Eigen::Vector3d;

class app : public game::engine
{
  public:
    app() : game::engine("mesh_separation")
    {
      // set camera position
      camera_up_->set_translation(glm::vec3{0.f, -5.f, -10.f});
      // add light
      auto light = hnll::game::actor::create();
      auto light_component = hnll::game::point_light_component::create(light, 100.f);
      add_point_light(light, light_component);
      light->set_translation({0.f, -20.f, 0.f});

      auto plane_geometry = create_plane_mesh();
      auto plane_graphics = graphics::mesh_model::create_from_geometry_mesh_model(get_graphics_device(), plane_geometry);
      auto plane = game::actor::create();
      auto plane_mesh_comp = game::mesh_component::create(plane, std::move(plane_graphics));
      game::engine::add_actor(plane);

      auto bunny_geometry = geometry::mesh_model::create_from_obj_file("bunny.obj");
      auto bunny_graphics = graphics::mesh_model::create_from_geometry_mesh_model(get_graphics_device(), bunny_geometry);
      auto bunny = game::actor::create();
      auto bunny_mesh_comp = game::mesh_component::create(bunny, std::move(bunny_graphics));
      game::engine::add_actor(bunny);
      bunny->set_rotation({M_PI, 0.f, 0.f});
    }
    ~app(){}

    s_ptr<geometry::mesh_model> create_plane_mesh()
    {
      auto v0 = geometry::vertex::create(vec3(-1.f, 0.f, 1.f));
      auto v1 = geometry::vertex::create(vec3(-1.f, 0.f, -1.f));
      auto v2 = geometry::vertex::create(vec3(1.f, 0.f, -1.f));
      auto v3 = geometry::vertex::create(vec3(1.f, 0.f, 1.f));
      v0->normal_ = { 0.f, -1.f, 0.f };
      v1->normal_ = { 0.f, -1.f, 0.f };
      v2->normal_ = { 0.f, -1.f, 0.f };
      v3->normal_ = { 0.f, -1.f, 0.f };
      auto model = geometry::mesh_model::create();
      model->add_face(v0, v1, v2);
      model->add_face(v0, v2, v3);
      return model;
    }
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