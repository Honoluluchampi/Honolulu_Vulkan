#pragma once

//hnll
#include <gui/engine.hpp>
#include <game/modules/graphics_engine.hpp>
#include <graphics/mesh_model.hpp>

// lib
#include <GLFW/glfw3.h>

//std
#include <chrono>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

namespace hnll {

namespace graphics {
  class meshlet_model;
  class skinning_mesh_model;
  class frame_anim_mesh_model;
  class frame_anim_meshlet_model;
}

namespace game {

// forward declaration
class actor;
class shading_system;
class default_camera;
class point_light_manager;
class point_light_component;
class physics_engine;

using actor_id = unsigned int;
using actor_map = std::unordered_map<actor_id, s_ptr<actor>>;

// TODO : use template
template <class T>
using graphics_model_map = std::unordered_map<std::string, u_ptr<T>>;

class engine {
  public:
    engine(const char *windowName = "honolulu engine");

    virtual ~engine();

    // delete copy ctor
    engine(const engine &) = delete;

    engine &operator=(const engine &) = delete;

    bool initialize();

    void run();

    static void add_actor(const s_ptr<actor> &actor);
    void remove_actor(actor_id id);

    template <class ShadingSystem>
    static void check_and_add_shading_system(utils::shading_type type)
    {
      if (graphics_engine_->check_shading_system_exists(type))
        return;
      auto system = ShadingSystem::create(get_graphics_device());
      add_shading_system(std::move(system));
    }
    static void add_shading_system(u_ptr<shading_system>&& shading_system);

    void add_point_light(s_ptr<actor> &owner, s_ptr<point_light_component> &light_comp);

    // TODO : delete this func
    void add_point_light_without_owner(const s_ptr<point_light_component> &light_comp);

    void remove_point_light_without_owner(component_id id);

    // getter
    static graphics_engine  &get_graphics_engine() { return *graphics_engine_; }
    static graphics::device &get_graphics_device() { return graphics_engine_->get_device_r(); }
    static actor& get_active_actor(actor_id id) { return *active_actor_map_[id]; }

    static actor& get_pending_actor(actor_id id)  { return *pending_actor_map_[id]; }
    static graphics::mesh_model&    get_mesh_model(const std::string& model_name);
    static graphics::meshlet_model& get_meshlet_model(const std::string& model_name);
    static graphics::skinning_mesh_model& get_skinning_mesh_model(const std::string& model_name);
    static graphics::frame_anim_mesh_model& get_frame_anim_mesh_model(const std::string& model_name);
    static graphics::frame_anim_meshlet_model& get_frame_anim_meshlet_model(const std::string& model_name);
    // setter
    void set_frustum_info(utils::frustum_info&& _frustum_info);

#ifndef IMGUI_DISABLED

    u_ptr<hnll::gui::engine> &get_gui_engine_up() { return gui_engine_; }

#endif

    // move u_ptr<func> before add
    static void add_glfw_mouse_button_callback(u_ptr<std::function<void(GLFWwindow *, int, int, int)>> &&func);

    // TODO : implement as physics engine
    void re_update_actors();

  protected:
    // TODO : remove static
    static GLFWwindow *glfw_window_;
    // hge actors
    s_ptr<default_camera> camera_up_;
    s_ptr<point_light_manager> light_manager_up_;

  private:
    inline void set_glfw_window() { glfw_window_ = graphics_engine_->get_glfw_window(); }

    void cleanup();

    void process_input();

    void update();

    // engine spacific update
    virtual void update_game(float dt) {}

    void render();

#ifndef IMGUI_DISABLED

    void update_gui();

    virtual void update_game_gui() {}

#endif

    // init
    virtual void setup_shading_systems();

    void init_actors();

    void load_data();

    virtual void load_actor();

    void unload_data();

    // load all models in modleDir
    // use filenames as the key of the map
    void load_models();
    static void load_model(const std::string& model_name, utils::shading_type type);

    // glfw
    static void set_glfw_mouse_button_callbacks();

    static void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

    // actors
    static actor_map active_actor_map_;
    static actor_map pending_actor_map_;
    static std::vector<actor_id> dead_actor_ids_;

    // modules
    static u_ptr<graphics_engine> graphics_engine_;
    u_ptr<physics_engine>         physics_engine_;

#ifndef IMGUI_DISABLED
    u_ptr<hnll::gui::engine>      gui_engine_;
#endif

    // map of graphics_model contains raw vulkan buffer of it
    static graphics_model_map<graphics::mesh_model>               mesh_model_map_;
    static graphics_model_map<graphics::meshlet_model>            meshlet_model_map_;
    static graphics_model_map<graphics::skinning_mesh_model>      skinning_mesh_model_map_;
    static graphics_model_map<graphics::frame_anim_mesh_model>    frame_anim_mesh_model_map_;
    static graphics_model_map<graphics::frame_anim_meshlet_model> frame_anim_meshlet_model_map_;

    bool is_updating_ = false; // for update
    bool is_running_ = false; // for run loop

    std::chrono::system_clock::time_point current_time_;

    // for rendering systems
    utils::viewer_info  viewer_info_;
    utils::frustum_info frustum_info_;

    // glfw
    static std::vector<u_ptr<std::function<void(GLFWwindow *, int, int, int)>>>
        glfw_mouse_button_callbacks_;
};

}} // namespace hnll::game
