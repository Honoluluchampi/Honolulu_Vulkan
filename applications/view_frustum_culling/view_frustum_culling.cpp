// hnll
#include <game/engine.hpp>
#include <game/actors/default_camera.hpp>
#include <game/actors/virtual_camera.hpp>
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
#include <graphics/frame_anim_meshlet_model.hpp>
#include <graphics/meshlet_utils.hpp>
#include <graphics/skinning_mesh_model.hpp>
#include <graphics/descriptor_set.hpp>

#include <game/shading_systems/mesh_shading_system.hpp>
#include <game/shading_systems/wire_frustum_shading_system.hpp>

#define FILENAME "/home/honolulu/models/characters/bunny.obj"
#define GLB_FILENAME "armadillo4896.glb"

namespace hnll {

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
    void sphere_multiply(float value)
    {
      auto current = bounding_volume_->get_sphere_radius();
      bounding_volume_->set_sphere_radius(current * value);
    }
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
        raw_meshlet_models_.push_back(ml_graphics);
        auto ml_mesh_comp = game::mesh_component::create(ml_actor, ml_graphics);
        game::engine::add_actor(ml_actor);
        ml_actor->set_rotation({M_PI, 0.f, 0.f});
        meshlet_actors_.emplace_back(std::move(ml_actor));
        set_translation(translation);
      }
    }

    // getter
    std::vector<s_ptr<meshlet_actor>>& get_meshlet_actors_ref() { return meshlet_actors_; }
  private:
    std::vector<s_ptr<graphics::mesh_model>> raw_meshlet_models_;
    std::vector<s_ptr<meshlet_actor>> meshlet_actors_;
};

class frame_meshlet_owner : public game::actor
{
  public:
    void add_separated_object(
      std::vector<std::vector<std::shared_ptr<geometry::mesh_model>>>& frame_meshlets,
      glm::vec3 translation,
      graphics::device& device)
    {
      set_transform(std::make_shared<utils::transform>());
      // for transform sharing
      auto tf = get_transform_sp();

      auto frame_count = frame_meshlets.size();
      frame_meshlet_actors_.resize(frame_count);

      for (int i = 0; i < frame_count; i++) {
        auto& meshlets = frame_meshlets[i];
        for (const auto &ml: meshlets) {
          auto ml_actor = std::make_shared<meshlet_actor>();
          ml_actor->set_bounding_volume(ml->get_bounding_volume_copy());
          ml_actor->share_transform(tf);
          auto ml_graphics = graphics::mesh_model::create_from_geometry_mesh_model(device, ml);
          raw_meshlet_models_.push_back(ml_graphics);
          auto ml_mesh_comp = game::mesh_component::create(ml_actor, ml_graphics);
          game::engine::add_actor(ml_actor);
          ml_actor->set_rotation({M_PI, 0.f, 0.f});
          ml_actor->set_scale({0.4f, 0.4f, 0.4f});
          ml_actor->sphere_multiply(0.4f);
          frame_meshlet_actors_[i].emplace_back(std::move(ml_actor));
          set_translation(translation);
        }
      }
    }

    void rotate_actors(const glm::vec3& rotation)
    {
      for (auto& meshlets : frame_meshlet_actors_) {
        for (auto& meshlet : meshlets) {
          meshlet->set_rotation(rotation);
        }
      }
    }

    // getter
    std::vector<s_ptr<meshlet_actor>>& get_meshlet_actors_ref(int frame_index) { return frame_meshlet_actors_[frame_index]; }
  private:
    std::vector<s_ptr<graphics::mesh_model>> raw_meshlet_models_;

    std::vector<std::vector<s_ptr<meshlet_actor>>> frame_meshlet_actors_;
};

class view_frustum_culling : public game::engine
{
  public:
    std::vector<glm::vec3> translations{
        {5.f,   0.f,   10.f},
        {-5.f,  0.f,   10.f},
        {0.f,   6.f,   10.f},
        {0.f,   -4.f,   10.f},
    };

    view_frustum_culling() : game::engine("view_frustum_culling")
    {
      add_virtual_camera();

      auto system = game::mesh_shading_system::create(get_graphics_device());
      add_shading_system(std::move(system));
      auto f_system = game::wire_frustum_shading_system::create(get_graphics_device());
      add_shading_system(std::move(f_system));

//      auto geometry = geometry::mesh_model::create_from_obj_file(FILENAME);
//      auto meshlets = geometry::mesh_separation::separate_into_raw(geometry, "bunny", geometry::mesh_separation::criterion::MINIMIZE_BOUNDING_SPHERE);

      auto& original = game::engine::get_skinning_mesh_model(GLB_FILENAME);
      auto ret = std::make_unique<graphics::frame_anim_meshlet_model>(original.get_device());
      ret->load_from_skinning_mesh_model(original, 20);

      auto geometry_models = geometry::mesh_model::create_from_dynamic_attributes(
        ret->get_ownership_of_raw_dynamic_attribs(),
        ret->get_raw_indices()
      );
      auto frame_meshlets = geometry::mesh_separation::separate_into_raw_frame(geometry_models);

      frame_count_ = frame_meshlets.size();

      for (const auto& translation : translations) {
        auto meshlet_owner = std::make_shared<frame_meshlet_owner>();
        meshlet_owner->add_separated_object(frame_meshlets, translation, get_graphics_device());
        frame_meshlet_owners_.emplace_back(std::move(meshlet_owner));
      }
    }

    void update_game(float dt) override
    {
      fps_ = 1.f / dt;

      if (increment_frame_) {
        current_frame_++;
        current_frame_ %= frame_count_;
      }

      // TODO : auto-update
      virtual_camera_->update_frustum_planes();
      set_frustum_info(virtual_camera_->get_frustum_info());

      // virtual frustum culling
      active_triangle_count_ = 0;
      whole_triangle_count_  = 0;
      const auto& frustum = virtual_camera_->get_perspective_frustum();
      for (auto& owner : frame_meshlet_owners_) {
        for (auto& meshlet : owner->get_meshlet_actors_ref(current_frame_)) {
          const auto sphere = meshlet->get_bounding_volume();
          auto obj = dynamic_cast<hnll::game::mesh_component *>(&meshlet->get_renderable_component_r());
          if (geometry::intersection::test_sphere_frustum(sphere, frustum)) {
            obj->set_should_be_drawn();
            active_triangle_count_ += obj->get_face_count();
          }
          whole_triangle_count_ += obj->get_face_count();
        }
      }
    }

    void rotate_actors(const glm::vec3& rotation)
    {
      for (auto& owner : frame_meshlet_owners_) {
        owner->rotate_actors(rotation);
      }
    }

    void update_game_gui() override
    {
      ImGui::Begin("stats");

      if (ImGui::Button("start / stop animation")) {
        increment_frame_ = !increment_frame_;
      }

      if (ImGui::Button("next frame")) {
        current_frame_++;
        current_frame_ %= frame_count_;
      }

      ImGui::Text("active triangle count: %d", active_triangle_count_);
      ImGui::Text("whole triangle count: %d", whole_triangle_count_);
      ImGui::Text("active triangle percentage: %.f", float(active_triangle_count_) / float(whole_triangle_count_) * 100.f);
      ImGui::Text("frame count : %.d", frame_count_);
      ImGui::Text("fps : %.f", fps_);

      if (active_triangle_counts_.size() < frame_count_) {
        active_triangle_counts_.push_back(active_triangle_count_);
        active_triangle_count_sum_ += active_triangle_count_;
        if (active_triangle_counts_.size() == frame_count_) {
          active_triangle_count_mean_ =
            float(active_triangle_count_sum_) / float(frame_count_);
        }
      }
      else {
        ImGui::Text("active triangle count mean : %f", active_triangle_count_mean_);
        ImGui::Text("active triangle count percentage : %.f", float(active_triangle_count_mean_) / float(whole_triangle_count_));
      }

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

      if (ImGui::Button("reset mean")) {
        active_triangle_counts_.clear();
        active_triangle_counts_.resize(0);
        active_triangle_count_sum_ = 0;
      }



      if (ImGui::Button("front"))  { rotate_actors({M_PI, 0.f, 0.f}); }
      if (ImGui::Button("back"))   { rotate_actors({0.f, 0.f, 0.f}); }
      if (ImGui::Button("right"))  { rotate_actors({M_PI, M_PI / 2.f, 0.f}); }
      if (ImGui::Button("left"))   { rotate_actors({M_PI, -M_PI / 2.f, 0.f}); }
      if (ImGui::Button("top"))    { rotate_actors({1.5 * M_PI, 0.f, 0.f}); }
      if (ImGui::Button("bottom")) { rotate_actors({M_PI / 2.f, 0.f, 0.f}); }
      ImGui::End();
    }
  private:
    void add_virtual_camera()
    {
      virtual_camera_ = game::virtual_camera::create(get_graphics_engine());
      add_actor(virtual_camera_);
    }

    std::vector<s_ptr<frame_meshlet_owner>> frame_meshlet_owners_;
    s_ptr<game::virtual_camera> virtual_camera_;
    std::vector<uint32_t> active_triangle_counts_;
    std::vector<uint32_t> whole_triangle_counts_;
    uint32_t active_triangle_count_sum_ = 0;
    float active_triangle_count_mean_;
    unsigned active_triangle_count_;
    unsigned whole_triangle_count_;
    float fps_;
    int current_frame_ = 0;
    int frame_count_ = 0;
    bool increment_frame_ = true;
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