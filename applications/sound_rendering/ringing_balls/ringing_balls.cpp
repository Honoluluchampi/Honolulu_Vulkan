// hnll
#include <game/components/audio_component.hpp>
#include <game/actor.hpp>
#include <game/engine.hpp>
#include <game/components/rigid_component.hpp>
#include <geometry/bounding_volume.hpp>
#include <game/components/mesh_component.hpp>
#include <game/components/point_light_component.hpp>
#include <game/actors/default_camera.hpp>
#include <physics/engine.hpp>
#include <physics/collision_info.hpp>
#include <physics/collision_detector.hpp>

using namespace hnll;

class rigid_ball : public hnll::game::actor
{
  public:
      rigid_ball() : hnll::game::actor(){}

      static s_ptr<rigid_ball> create()
      {
          // create ball actor and its mesh
          auto ball = std::make_shared<rigid_ball>();
          auto ball_mesh = hnll::game::engine::get_mesh_model_sp("sphere");
          auto ball_mesh_vertex_position_list = ball_mesh->get_vertex_position_list();
          auto ball_mesh_comp = hnll::game::mesh_component::create(ball, std::move(ball_mesh));

          // create bounding_sphere
          auto bounding_sphere = hnll::geometry::bounding_volume::create_bounding_sphere
                  (hnll::geometry::bv_ctor_type::RITTER, ball_mesh_vertex_position_list);
          ball->rigid_component_ = game::rigid_component::create_from_bounding_volume(*ball, std::move(bounding_sphere));

          // register the ball to the engine
          hnll::game::engine::add_actor(ball);
          physics::collision_detector::add_rigid_component(ball->rigid_component_);
          return ball;
      };

      void assign_audio()
      {
        audio_component_ = game::audio_component::create();

        // raw audio creation
        const unsigned int freq = 44100;
        const float pitch = 440.0f;
        const float duration = 0.05f;
        std::vector<ALshort> audio(static_cast<size_t>(freq * duration));
        for (int i = 0; i < audio.size(); i++) {
          audio[i] = std::sin(pitch * M_PI * 2.0 * i / freq)
                     * std::numeric_limits<ALshort>::max();
        }

        audio_component_->set_raw_audio(std::move(audio));
        audio_component_->get_ready_to_play();
      }

      void init(const Eigen::Vector3d& center_point, double radius)
      {
        // init ball state
        position_ = glm::vec3{center_point.x(), center_point.y(), center_point.z()};
        set_translation(glm::vec3{center_point.x(), center_point.y(), center_point.z()});
        set_scale(glm::vec3(radius, radius, radius));
        velocity_ = {0.f, 0.f, 0.f};
      }

      void update_actor(float dt) override
      {
          position_ += velocity_ * dt;
          velocity_.y += gravity_ * dt;
          this->set_translation(position_);
      }

      // this update function is invoked if collision_detector detects collision with other component (plane in this situation)
      void re_update(const hnll::physics::collision_info& info) override
      {
          position_.y -= info.intersection_depth;
          velocity_.y = -velocity_.y * restitution_;
          if (std::abs(velocity_.y) < velocity_thresh_) velocity_.y = 0;
          else {
            audio_component_->play_sound();
            this->set_translation(position_);
          }
      }

  private:
      glm::vec3 position_;
      glm::vec3 velocity_;
      double mass_ = 3.f;
      double velocity_thresh_ = 1.3f;
      double gravity_ = 40.f;
      double restitution_ = 0.5;
      s_ptr<hnll::game::rigid_component> rigid_component_;
      u_ptr<game::audio_component> audio_component_;
};

// plate is bounding box of which thickness is 0.
class rigid_plane : public hnll::game::actor
{
  public:
      rigid_plane() : actor(){}
      static s_ptr<rigid_plane> create()
      {
          auto plane = std::make_shared<rigid_plane>();
          auto plane_mesh = hnll::game::engine::get_mesh_model_sp("big_plane");
          auto plane_mesh_vertices = plane_mesh->get_vertex_position_list();
  //        auto plane_mesh_comp = hnll::game::mesh_component::create(plane, std::move(plane_mesh));
          auto bounding_box = hnll::geometry::bounding_volume::create_aabb(plane_mesh_vertices);

          plane->rigid_component_ = game::rigid_component::create_from_bounding_volume(*plane, std::move(bounding_box));

          hnll::game::engine::add_actor(plane);
          physics::collision_detector::add_rigid_component(plane->rigid_component_);

          return plane;
      }
  private:
      s_ptr<game::rigid_component> rigid_component_;
      glm::vec3 position_;
};

class falling_ball_app : public hnll::game::engine
{
  public:
   std::vector<Eigen::Vector3d> position_list = {
       {0.f, -10.f, 0.f},
       {2.1f, -2.f, 0.f},
       {-2.1f, -7.f, 0.f}
   };
    falling_ball_app() : hnll::game::engine("falling ball")
    {
      audio::engine::start_hae_context();

      // set camera position
      camera_up_->set_translation(glm::vec3{0.f, 0.f, -20.f});

      // add light
      auto light = hnll::game::actor::create();
      auto light_component = hnll::game::point_light_component::create(light, 100.f);
      add_point_light(light, light_component);
      light->set_translation({-8.f, -20.f, -8.f});

      // add rigid ball
      for (const auto& position : position_list) {
        auto ball = rigid_ball::create();
        ball->init(position, 1.f);
        ball->assign_audio();
        balls_.emplace_back(std::move(ball));
      }

      // add plane
      auto rigid_plane = rigid_plane::create();
    }

    ~falling_ball_app() { audio::engine::kill_hae_context(); }

    void update_game_gui() override
    {
      ImGui::Begin("debug");

      if (ImGui::Button("restart")) {
        for(int i = 0; i < balls_.size(); i++) {
          balls_[i]->init(position_list[i], 1.f);
        }
      }
      ImGui::End();
    }

  private:
    std::vector<s_ptr<rigid_ball>> balls_;
    hnll::physics::engine physics_engine_{};
};

int main()
{
  falling_ball_app app{};
  try { app.run(); }
  catch (const std::exception& e) {
      std::cerr << e.what() << '\n';
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}