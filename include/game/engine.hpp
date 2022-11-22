#pragma once

//hnll
#include <gui/engine.hpp>
#include <graphics/engine.hpp>
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

namespace physics  { class engine; }
namespace graphics { class meshlet_model; }

namespace game {

// forward declaration
class actor;
class default_camera;
class point_light_manager;
class point_light_component;

using actor_id = unsigned int;
using actor_map = std::unordered_map<actor_id, s_ptr<actor>>;
using mesh_model_map = std::unordered_map<std::string, s_ptr<hnll::graphics::mesh_model>>;
using meshlet_model_map = std::unordered_map<std::string, u_ptr<hnll::graphics::meshlet_model>>;

class engine {
  public:
    engine(const char *windowName = "honolulu engine");

    virtual ~engine() = default;

    // delete copy ctor
    engine(const engine &) = delete;

    engine &operator=(const engine &) = delete;

    bool initialize();

    void run();

    static void add_actor(const s_ptr<actor> &actor);

    // void add_actor(s_ptr<actor>&& actor);
    void remove_actor(actor_id id);

    // takes s_ptr<renderable_component>
    template<class S>
    void set_renderable_component(S &&comp) { graphics_engine_->set_renderable_component(std::forward<S>(comp)); }

    template<class S>
    void replace_renderable_component(S &&comp) {
      graphics_engine_->replace_renderable_component(std::forward<S>(comp));
    }

    void remove_renderable_component(render_type type, component_id id) {
      graphics_engine_->remove_renderable_component_without_owner(type, id);
    }

    void add_point_light(s_ptr<actor> &owner, s_ptr<point_light_component> &light_comp);

    // TODO : delete this func
    void add_point_light_without_owner(const s_ptr<point_light_component> &light_comp);

    void remove_point_light_without_owner(component_id id);

    // getter
    hnll::graphics::engine &get_graphics_engine() { return *graphics_engine_; }
    hnll::graphics::device &get_graphics_device() { return graphics_engine_->get_device(); }
    static actor& get_active_actor(actor_id id)   { return *active_actor_map_[id]; }
    static actor& get_pending_actor(actor_id id)  { return *pending_actor_map_[id]; }
    static graphics::mesh_model&    get_mesh_model(std::string model_name) { return *mesh_model_map_[model_name]; }
    static graphics::meshlet_model& get_meshlet_model(std::string model_name);
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
    void init_actors();

    void load_data();

    virtual void load_actor();

    void unload_data();

    // load all models in modleDir
    // use filenames as the key of the map
    void load_mesh_models();
    void load_meshlet_models();

    // glfw
    static void set_glfw_mouse_button_callbacks();

    static void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

    static actor_map active_actor_map_;
    static actor_map pending_actor_map_;
    static std::vector<actor_id> dead_actor_ids_;

    // each engines
    u_ptr<hnll::graphics::engine> graphics_engine_;
    u_ptr<hnll::physics::engine>  physics_engine_;
#ifndef IMGUI_DISABLED
    u_ptr<hnll::gui::engine>      gui_engine_;
#endif

    // map of mesh_model contains raw vulkan buffer of its model
    // shared by engine and some modelComponents
    // pool all models which could be necessary
    static mesh_model_map    mesh_model_map_;
    static meshlet_model_map meshlet_model_map_;
    // map of mesh_model_info contains

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
