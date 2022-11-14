// hnll
#include <graphics/engine.hpp>
#include <graphics/utils.hpp>
#include <graphics/meshlet_model.hpp>
#include <game/actor.hpp>
#include <game/engine.hpp>
#include <game/actors/default_camera.hpp>
#include <game/actors/virtual_camera.hpp>
#include <game/components/meshlet_component.hpp>
#include <physics/engine.hpp>

// std
#include <memory>
#include <iostream>

namespace hnll {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;
using vec2 = Eigen::Vector2f;
using vec3 = Eigen::Vector3f;
using vec4 = Eigen::Vector4f;

class ml_actor : public game::actor
{
  public:
    static s_ptr<ml_actor> create(graphics::device& _device)
    {
      auto ret = std::make_shared<ml_actor>();
      std::string filename = "bunny.obj";
      auto raw_meshlet_model = graphics::meshlet_model::create_from_file(_device, filename);
      ret->meshlet_comp_ = game::meshlet_component::create(ret, std::move(raw_meshlet_model));
      ret->set_rotation({M_PI, 0.f, 0.f});
      game::engine::add_actor(ret);
      return ret;
    }
    ml_actor(){}
  private:
    s_ptr<game::meshlet_component> meshlet_comp_;
};

class mesh_shader_introduction : public game::engine
{
  public:
    mesh_shader_introduction() : game::engine("mesh shader introduction")
    {
      ml_actor_ = ml_actor::create(get_graphics_device());
      add_virtual_camera();
    }

    ~mesh_shader_introduction() override = default;

  private:
    void add_virtual_camera()
    {
      virtual_camera_ = game::virtual_camera::create(get_graphics_engine());
      add_actor(virtual_camera_);
    }

    void update_game_gui() override
    {
      ImGui::Begin("stats");

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
    // sample object
    s_ptr<ml_actor> ml_actor_;
    s_ptr<game::virtual_camera> virtual_camera_;
    // frustum culling is organized based on this frustum;
    s_ptr<geometry::perspective_frustum> active_frustum_;
};
} // namespace hnll

int main() {
  hnll::mesh_shader_introduction app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}