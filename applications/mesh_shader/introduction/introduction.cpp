// hnll
#include <graphics/engine.hpp>
#include <graphics/utils.hpp>
#include <graphics/meshlet_model.hpp>
#include <game/actor.hpp>
#include <game/engine.hpp>
#include <game/actors/default_camera.hpp>
#include <game/actors/virtual_camera.hpp>
#include <game/components/mesh_component.hpp>
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

std::string FILENAME = "light_bunny.obj";

class ml_actor : public game::actor
{
  public:
    static s_ptr<ml_actor> create(graphics::device& _device)
    {
      auto ret = std::make_shared<ml_actor>();
      ret->meshlet_comp_ = game::meshlet_component::create(ret, FILENAME);
      ret->set_rotation({M_PI, 0.f, 0.f});
      game::engine::add_actor(ret);
      return ret;
    }
    ml_actor(){}
  private:
    s_ptr<game::meshlet_component> meshlet_comp_;
};

class mesh_actor : public game::actor
{
  public:
    static s_ptr<mesh_actor> create(graphics::device& _device)
    {
      auto ret = std::make_shared<mesh_actor>();
      ret->mesh_comp_ = game::mesh_component::create(ret, FILENAME);
      ret->set_rotation({M_PI, 0.f, 0.f});
      game::engine::add_actor(ret);
      return ret;
    }
  private:
    s_ptr<game::mesh_component> mesh_comp_;
};

class mesh_shader_introduction : public game::engine
{
  public:
    mesh_shader_introduction() : game::engine("mesh shader introduction")
    {
      create_bunny_wall<mesh_actor>();
      add_virtual_camera();
    }

    ~mesh_shader_introduction() override = default;

  private:
    void add_virtual_camera()
    {
      virtual_camera_ = game::virtual_camera::create(get_graphics_engine());
      add_actor(virtual_camera_);
    }

    void update_game(float dt) { fps_ = 1.0f / dt; }

    template <class T>
    void create_bunny_wall()
    {
      uint32_t x_count = 4;
      uint32_t y_count = 4;
      uint32_t z_count = 4;
      float space = 4.f;
      std::vector<vec3> positions;

      for (int i = 0; i < x_count; i++) {
        for (int j = 0; j < y_count; j++) {
          for (int k = 0; k < z_count; k++) {
            if (i != 0 && i != x_count - 1 && j != 0 && j != y_count - 1 && k != 0 && k != z_count - 1)
              continue;
            glm::vec3 position = {
              (i - (x_count / 2.f)) * space,
              (j - (y_count / 2.f)) * space,
              (k - (z_count / 2.f)) * space
            };
            auto object = T::create(get_graphics_device());
            object->set_translation(position);
          }
        }
      }
    }

    void update_game_gui() override
    {
      // this part should be contained in update_game()...
      virtual_camera_->update_frustum_planes();
      set_frustum_info(virtual_camera_->get_frustum_info());

      ImGui::Begin("stats");

      ImGui::Text("fps : %.f", fps_);

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
    std::vector<s_ptr<ml_actor>> ml_actors_;

    s_ptr<game::virtual_camera> virtual_camera_;
    // frustum culling is organized based on this frustum;
    s_ptr<geometry::perspective_frustum> active_frustum_;
    float fps_;
};
} // namespace hnll

int main() {
  hnll::mesh_shader_introduction app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}