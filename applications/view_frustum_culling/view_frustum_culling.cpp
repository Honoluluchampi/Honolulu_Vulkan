// hnll
#include <game/engine.hpp>
#include <game/actors/default_camera.hpp>
#include <game/components/viewer_component.hpp>
#include <game/components/rigid_component.hpp>
#include <game/components/mesh_component.hpp>
#include <game/components/point_light_component.hpp>
#include <game/components/wire_frame_frustum_component.hpp>
#include <game/components/keyboard_movement_component.hpp>
#include <geometry/mesh_model.hpp>
#include <geometry/mesh_separation.hpp>
#include <geometry/perspective_frustum.hpp>
#include <geometry/bounding_volume.hpp>
#include <geometry/intersection.hpp>

namespace hnll {

class virtual_camera : public game::actor
{
  public:
    static s_ptr<virtual_camera> create(hnll::graphics::engine& engine, graphics::device& device)
    {
      auto camera = std::make_shared<virtual_camera>();
      camera->viewer_comp_ = game::viewer_component::create(*camera->get_transform_sp(), engine.get_renderer());
      camera->viewer_comp_->auto_update_view_frustum();
      auto frustum = geometry::perspective_frustum::create(M_PI / 4.f, M_PI / 4.f, 1.f, 15.f);
      camera->wire_frustum_comp_ = game::wire_frame_frustum_component::create(camera, frustum, device);
      camera->key_comp_ = std::make_shared<game::keyboard_movement_component>(engine.get_window().get_glfw_window(), *camera->get_transform_sp());
      camera->add_component(camera->key_comp_);
      camera->key_comp_->set_updating_off();
      return camera;
    }
    virtual_camera() = default;
    ~virtual_camera() = default;

    void update_frustum_planes()
    {
      wire_frustum_comp_->update_frustum_planes(*get_transform_sp());
    }

    // getter
    const geometry::perspective_frustum& get_perspective_frustum() { return wire_frustum_comp_->get_perspective_frustum(); }

    // setter
    void set_movement_updating_on()  { key_comp_->set_updating_on(); }
    void set_movement_updating_off() { key_comp_->set_updating_off(); }
  private:
    s_ptr<game::viewer_component> viewer_comp_;
    s_ptr<game::wire_frame_frustum_component> wire_frustum_comp_;
    s_ptr<game::keyboard_movement_component> key_comp_;
};

class meshlet_actor : public game::actor
{
  public:
    meshlet_actor() : game::actor()
    {}

    // getter
    const geometry::bounding_volume get_bounding_volume() const { return *bounding_volume_; }
    // setter
    void set_bounding_volume(u_ptr<geometry::bounding_volume>&& bv) { bounding_volume_ = std::move(bv); }
    void share_transform(s_ptr<utils::transform>& tf) { set_transform(tf); bounding_volume_->set_transform(tf); }

  private:
    u_ptr<geometry::bounding_volume> bounding_volume_;
};

class meshlet_owner : public game::actor
{
  public:
    void add_separated_object(std::vector<std::shared_ptr<geometry::mesh_model>>& meshlets, glm::vec3 translation, graphics::device& device)
    {
      set_transform(std::make_shared<utils::transform>());
      // for transform sharing
      auto tf = get_transform_sp();

      for (const auto &ml: meshlets) {
        auto ml_actor = std::make_shared<meshlet_actor>();
        ml_actor->set_bounding_volume(ml->get_bounding_volume_copy());
        ml_actor->share_transform(tf);
        auto ml_graphics = graphics::mesh_model::create_from_geometry_mesh_model(device, ml);
        auto ml_mesh_comp = game::mesh_component::create(ml_actor, std::move(ml_graphics));
        game::engine::add_actor(ml_actor);
        ml_actor->set_rotation({M_PI, 0.f, 0.f});
        meshlet_actors_.emplace_back(std::move(ml_actor));
        set_translation(translation);
      }
    }

    // getter
    std::vector<s_ptr<meshlet_actor>>& get_meshlet_actors_ref() { return meshlet_actors_; }
  private:
    std::vector<s_ptr<meshlet_actor>> meshlet_actors_;
};

class view_frustum_culling : public game::engine
{
  public:
    std::vector<glm::vec3> translations{
        {5.f,   0.f,   10.f},
        {-5.f,  0.f,   10.f},
        {0.f,   6.f,   10.f},
        {0.f,   -4.f,   10.f},

//        {5.f,   2.5f,  10.f},
//        {5.f,   5.f,   10.f},
//        {5.f,   -2.5f, 10.f},
//        {-5.f,  2.5f,  10.f},
//        {-5.f,  5.f,   10.f},
//        {-5.f,  -2.5f, 10.f},
//        {2.5f,  6.f,   10.f},
//        {-2.5f, 6.f,   10.f},
//        {2.5f,  -4.f,   10.f},
//        {-2.5f, -4.f,   10.f},
    };

    view_frustum_culling() : game::engine("view_frustum_culling")
    {
      auto geometry = geometry::mesh_model::create_from_obj_file("light_bunny.obj");
      auto meshlets = geometry::mesh_separation::separate(geometry);

      for (const auto& translation : translations) {
        auto meshlet_owne = std::make_shared<meshlet_owner>();
        meshlet_owne->add_separated_object(meshlets, translation, get_graphics_device());
        meshlet_owners_.emplace_back(std::move(meshlet_owne));
      }
      add_virtual_camera();
      setup_lights();
    }

    void update_game(float dt) override
    {
      // TODO : auto-update
      virtual_camera_->update_frustum_planes();

      // virtual frustum culling
      active_triangle_count_ = 0;
      const auto& frustum = virtual_camera_->get_perspective_frustum();
      for (auto& owner : meshlet_owners_) {
        for (auto& meshlet : owner->get_meshlet_actors_ref()) {
          const auto &sphere = meshlet->get_bounding_volume();
          auto obj = dynamic_cast<hnll::game::mesh_component *>(meshlet->get_renderable_component_sp().get());
          if (!geometry::intersection::test_sphere_frustum(sphere, frustum)) {
            obj->set_should_not_be_drawn();
          }
          else active_triangle_count_ += obj->get_face_count();
        }
      }
    }

    void update_game_gui() override
    {
      ImGui::Begin("d");
        ImGui::Text("active triangle count: %d", active_triangle_count_);

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
      float light_intensity = 80.f;
      std::vector<glm::vec3> positions;

      positions.emplace_back(glm::vec3{0.f, 0.f, 0.f});
      for (const auto& position : positions) {
        auto light = hnll::game::actor::create();
        auto light_component = hnll::game::point_light_component::create(light, light_intensity, 0.f);
        add_point_light(light, light_component);
        light->set_translation(position);
      }
    };

    s_ptr<virtual_camera> virtual_camera_;
    std::vector<s_ptr<meshlet_owner>> meshlet_owners_;
    unsigned active_triangle_count_;
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