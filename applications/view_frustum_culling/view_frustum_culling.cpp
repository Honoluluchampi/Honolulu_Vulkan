// hnll
#include <game/engine.hpp>
#include <game/components/viewer_component.hpp>
#include <game/components/rigid_component.hpp>
#include <game/components/point_light_component.hpp>
#include <game/components/wire_frame_frustum_component.hpp>
#include <geometry/mesh_separation.hpp>

namespace hnll {

class virtual_camera : public game::actor
{
  public:
    static s_ptr<virtual_camera> create(hnll::graphics::renderer& renderer, graphics::device& device)
    {
      auto camera = std::make_shared<virtual_camera>();
      camera->viewer_comp_ = game::viewer_component::create(*camera->get_transform_sp(), renderer);
      camera->viewer_comp_->auto_update_view_frustum();
      auto frustum = geometry::perspective_frustum::create(M_PI / 4.f, M_PI / 4.f, 5.f, 2.f);
      camera->wire_frustum_comp_ = game::wire_frame_frustum_component::create(camera, frustum, device);
      return camera;
    }
    virtual_camera() = default;
    ~virtual_camera() = default;
  private:
    s_ptr<game::viewer_component> viewer_comp_;
    s_ptr<game::wire_frame_frustum_component> wire_frustum_comp_;
};

class view_frustum_culling : public game::engine
{
  public:
    view_frustum_culling() : game::engine("view_frustum_culling")
    {
      add_virtual_camera();
      setup_lights();
    }

    void update_game(float dt) override
    {

    }
  private:
    void add_virtual_camera()
    {
      auto virtual_camera = virtual_camera::create(get_graphics_engine().get_renderer(), get_graphics_device());
      add_actor(virtual_camera);
    }

    void setup_lights()
    {
      float light_intensity = 4.f;
      std::vector<glm::vec3> positions;
      float position_radius = 4.f;
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

    void add_separated_model(const std::string& filename)
    {

    }
};

}

int main()
{
  hnll::view_frustum_culling app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}