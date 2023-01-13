// hnll
#include <game/engine.hpp>
#include <game/shading_systems/frame_anim_mesh_shading_system.hpp>
#include <game/components/mesh_component.hpp>
#include <game/components/skinning_mesh_component.hpp>
#include <game/components/frame_anim_component.hpp>
#include <game/components/point_light_component.hpp>

// std
#include <iostream>

namespace hnll {

class gltf_animation : public game::engine
{
  public:
    gltf_animation() : game::engine("gltf animation")
    {
      add_model();
    }

  private:

    void add_model()
    {
      std::string model_name = "armagilo.glb";

      auto actor = game::actor::create();
      auto obj = game::skinning_mesh_component::create(actor, model_name);
      actor->set_scale({0.3f, 0.3f, 0.3f});
      actor->set_rotation({M_PI, 0.f, 0.f});
      actor->set_translation({3.f, 0.f, 0.f});
//
      auto frame_actor = game::actor::create();
//      auto frame_mesh = game::mesh_component::create(frame_actor, model_name);
      auto frame_mesh = game::frame_anim_component<graphics::frame_anim_mesh_model>::create(frame_actor, model_name);
      frame_actor->set_scale({0.3f, 0.3f, 0.3f});
      frame_actor->set_rotation({M_PI, 0.f, 0.f});
      frame_actor->set_translation({-3.f, 0.f, 0.f});
    }

    void update_game(float dt) override
    {
      fps_ = 1.f / dt;
    }

    void update_game_gui() override
    {
      ImGui::Begin("stats");

      ImGui::Text("fps : %.f", fps_);

      ImGui::End();
    }

    float fps_;
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