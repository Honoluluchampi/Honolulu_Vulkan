#pragma once

//hnll
#include <game/actor.hpp>
#include <game/components/mesh_component.hpp>
#include <game/actors/default_camera.hpp>
#include <game/actors/point_light_manager.hpp>
#include <gui/engine.hpp>
#include <graphics/engine.hpp>

// lib
#include <GLFW/glfw3.h>
// #include <X11/extensions/XTest.h>

//std
#include <chrono>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

namespace hnll {
namespace game {

class game
{
public:
  game(const char* windowName = "honolulu engine");
  ~game();
  // delete copy ctor
  game(const game &) = delete;
  game& operator=(const game &) = delete;

  bool initialize();
  void run();

  void add_actor(const s_ptr<actor>& actor);
  // void add_actor(s_ptr<actor>&& actor);
  void remove_actor(actor::id id);

  // factory funcs
  // takes hgeActor derived class as template argument
  template<class ActorClass = actor, class... Args>
  static s_ptr<ActorClass> create_actor(Args... args)
  {
    auto actor = std::make_shared<ActorClass>(args...);
    // create s_ptr of actor perform as actor
    std::shared_ptr<actor> prt_for_actor_map = actor;
    // register it to the actor map
    pending_actor_map_.emplace(prt_for_actor_map->get_id(), prt_for_actor_map);
    return actor;
  }

  // takes s_ptr<renderable_component>
  template <class S>
  void set_renderable_component(S&& comp) { graphics_engine_up_->set_renderable_component(std::forward<S>(comp)); }
  template <class S> 
  void replace_renderable_component(S&& comp) { graphics_engine_up_->replace_renderable_component(std::forward<S>(comp)); }
  void remove_renderable_component(render_type type, component::id id) { graphics_engine_up_->remove_renderable_component_without_owner(type, id); }

  void add_point_light(s_ptr<actor>& owner, s_ptr<point_light_component>& light_comp);
  // TODO : delete this func
  void add_point_light_without_owner(s_ptr<point_light_component>& light_comp);
  void remove_point_light_without_owner(component::id id);

  void set_camera_transform(const hnll::utils::transform& transform){ camera_up_->get_transform() = transform; }

  // getter
  hnll::graphics::engine& get_graphics_engine() { return *graphics_engine_up_; }
  hnll::graphics::device& get_graphics_device() { return graphics_engine_up_->get_device(); }
  s_ptr<hnll::graphics::mesh_model> get_mesh_model_sp(std::string model_name) { return mesh_model_map_[model_name]; }

#ifndef IMGUI_DISABLED
  u_ptr<hnll::gui::engine>& get_gui_engine_up() { return gui_engine_up_; }
#endif
  // move u_ptr<func> before add
  static void add_glfw_mouse_button_callback(u_ptr<std::function<void(GLFWwindow*, int, int, int)>>&& func);

  // X11
  // static Display* x11Display() { return display_; }
protected:
  // TODO : remove static
  static GLFWwindow* glfwWindow_m;
  // hge actors
  s_ptr<default_camera> camera_up_;
  s_ptr<point_light_manager> upLightManager_;

private:
  inline void set_glfw_window() { glfwWindow_m = graphics_engine_up_->get_glfw_window() ; }
  void cleanup();
  void process_input();
  void update();
  // game spacific update
  virtual void update_game(float dt){}
  void render();

#ifndef IMGUI_DISABLED
  void update_gui();
  virtual void update_game_gui(){}
#endif

  // init 
  void init_actors();
  void load_data();
  virtual void load_actor();

  void unload_data();
  // load all models in modleDir
  // use filenames as the key of the map
  void load_mesh_models(const std::string& modelDir = "/models");

  // glfw
  static void set_glfw_mouse_button_callbacks();
  static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

  actor::map active_actor_map_;
  static actor::map pending_actor_map_;
  actor::map dead_actor_map_;

  u_ptr<hnll::graphics::engine> graphics_engine_up_;

#ifndef IMGUI_DISABLED
  u_ptr<hnll::gui::engine> gui_engine_up_;
#endif

  // map of mesh_model
  // shared by game and some modelComponents
  // pool all models which could be necessary
  hnll::graphics::mesh_model::map mesh_model_map_;

  bool is_updating_ = false; // for update
  bool is_running_ = false; // for run loop

  std::chrono::system_clock::time_point current_time_;

  // temp
  actor::id hieModelID_;

  // glfw
  static std::vector<u_ptr<std::function<void(GLFWwindow*, int, int, int)>>> 
    glfw_mouse_button_callbacks_;

  // X11
  // static Display* display_;
};

} // namespace game
} // namespace hnll