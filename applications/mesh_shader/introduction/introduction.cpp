// hnll
#include <graphics/utils.hpp>
#include <game/actor.hpp>
#include <game/engine.hpp>
#include <game/actors/default_camera.hpp>
#include <game/actors/virtual_camera.hpp>
#include <game/components/mesh_component.hpp>
#include <game/components/meshlet_component.hpp>
#include <game/components/frame_anim_component.hpp>
#include <game/shading_systems/wire_frustum_shading_system.hpp>
#include <physics/engine.hpp>

#include <game/shading_systems/mesh_shading_system.hpp>

// std
#include <memory>
#include <iostream>

namespace hnll {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;
using vec2 = Eigen::Vector2f;
using vec3 = Eigen::Vector3f;
using vec4 = Eigen::Vector4f;

std::string FILENAME = "bunny.obj";
#define MODEL_SCALE 0.5f

template <class ModelComp>
class model_actor : public game::actor
{
  public:
    static s_ptr<model_actor> create(graphics::device& _device)
    {
      auto ret = std::make_shared<model_actor>();
      ret->model_comp_ = ModelComp::create(ret, FILENAME);
      ret->set_rotation({M_PI, 0.f, 0.f});
      ret->set_scale({MODEL_SCALE, MODEL_SCALE, MODEL_SCALE});
      game::engine::add_actor(ret);
      return ret;
    }
    uint32_t get_meshlet_count() { return model_comp_->get_meshlet_count(); }
    model_actor(){}
  private:
    s_ptr<ModelComp> model_comp_;
};

template<>
uint32_t model_actor<game::mesh_component>::get_meshlet_count() { return 1; }

class mesh_shader_introduction : public game::engine
{
  public:
    mesh_shader_introduction() : game::engine("mesh shader introduction")
    {
      auto system = game::wire_frustum_shading_system::create(get_graphics_device());
      game::engine::add_shading_system(std::move(system));

      // mesh_actor or ml_actor
      create_bunny_wall<model_actor<game::mesh_component>>();
      add_virtual_camera();
    }

    ~mesh_shader_introduction() override = default;

  private:
    void add_virtual_camera()
    {
      virtual_camera_ = game::virtual_camera::create(get_graphics_engine());
      add_actor(virtual_camera_);
    }

    void update_game(float dt)
    {
      float sampling_fps = 2.f;
      static float acc_fps = 0.0f;
      static int acc_count = 0;
      static float duration = 0.f;

      acc_fps += 1.f / dt;
      acc_count++;
      duration += dt;

      if (duration > 1.f / sampling_fps) {
        fps_ = acc_fps / acc_count;
        acc_fps = 0.f;
        acc_count = 0;
        duration = 0.f;
      }
    }

    template <class T>
    void create_bunny_wall()
    {
      uint32_t x_count = 4;
      uint32_t y_count = 4;
      uint32_t z_count = 4;
      float space = 2.f;
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
            if (meshlet_count_ == 0)
              meshlet_count_ = object->get_meshlet_count();
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
      ImGui::Text("meshlet count : %.d", meshlet_count_);

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

    s_ptr<game::virtual_camera> virtual_camera_;
    // frustum culling is organized based on this frustum;
    s_ptr<geometry::perspective_frustum> active_frustum_;
    float fps_;
    uint32_t meshlet_count_ = 0;
};
} // namespace hnll

int main() {
  hnll::mesh_shader_introduction app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}