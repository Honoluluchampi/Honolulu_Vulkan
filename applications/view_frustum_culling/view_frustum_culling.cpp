// hnll
#include <game/engine.hpp>
#include <game/actors/default_camera.hpp>
#include <game/components/viewer_component.hpp>
#include <game/components/rigid_component.hpp>
#include <game/components/point_light_component.hpp>
#include <game/components/wire_frame_frustum_component.hpp>
#include <game/components/keyboard_movement_component.hpp>
#include <geometry/mesh_separation.hpp>

namespace hnll {

class virtual_camera : public game::actor
{
  public:
    static s_ptr<virtual_camera> create(hnll::graphics::engine& engine, graphics::device& device)
    {
      auto camera = std::make_shared<virtual_camera>();
      camera->viewer_comp_ = game::viewer_component::create(*camera->get_transform_sp(), engine.get_renderer());
      camera->viewer_comp_->auto_update_view_frustum();
      auto frustum = geometry::perspective_frustum::create(M_PI / 4.f, M_PI / 4.f, 5.f, 0.2f);
      camera->wire_frustum_comp_ = game::wire_frame_frustum_component::create(camera, frustum, device);
      camera->key_comp_ = std::make_shared<game::keyboard_movement_component>(engine.get_window().get_glfw_window(), *camera->get_transform_sp());
      camera->add_component(camera->key_comp_);
      camera->key_comp_->set_updating_off();
      return camera;
    }
    virtual_camera() = default;
    ~virtual_camera() = default;

    void set_movement_updating_on()  { key_comp_->set_updating_on(); }
    void set_movement_updating_off() { key_comp_->set_updating_off(); }
  private:
    s_ptr<game::viewer_component> viewer_comp_;
    s_ptr<game::wire_frame_frustum_component> wire_frustum_comp_;
    s_ptr<game::keyboard_movement_component> key_comp_;
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

    void update_game_gui() override
    {
      ImGui::Begin("d");
      if (ImGui::Button("change key move target")) {
        if (camera_up_->is_movement_updating()) {
          camera_up_->set_movement_updating_off();
          virtual_camera_->set_movement_updating_on();
        }
        else {
          camera_up_->set_movement_updating_on();
          virtual_camera_->set_movement_updating_off();
        }
      }
      ImGui::End();
    }
  private:
    void add_virtual_camera()
    {
      virtual_camera_ = virtual_camera::create(get_graphics_engine(), get_graphics_device());
      add_actor(virtual_camera_);
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
    s_ptr<virtual_camera> virtual_camera_;
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