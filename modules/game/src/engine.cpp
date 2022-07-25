// hnll
#include <game/engine.hpp>
#include <game/actor.hpp>
#include <game/actors/point_light_manager.hpp>
#include <game/actors/default_camera.hpp>
#include <game/components/mesh_component.hpp>

// lib
#include <imgui.h>

// std
#include <filesystem>
#include <iostream>
#include <typeinfo>

namespace hnll {
namespace game {

constexpr float MAX_FPS = 30.0f;
constexpr float MAX_DT = 0.05f;

// static members
actor_map engine::pending_actor_map_;
mesh_model_map engine::mesh_model_map_;
// glfw
GLFWwindow* engine::glfw_window_;
std::vector<u_ptr<std::function<void(GLFWwindow*, int, int, int)>>> engine::glfw_mouse_button_callbacks_{};

// x11
// Display* engine::display_ = XOpenDisplay(NULL);

engine::engine(const char* window_name) : graphics_engine_up_(std::make_unique<hnll::graphics::engine>(window_name))
{
  set_glfw_window(); // ?

#ifndef IMGUI_DISABLED
  gui_engine_up_ = std::make_unique<hnll::gui::engine>
    (graphics_engine_up_->get_window(), graphics_engine_up_->get_device());
  // configure dependency between renderers
  graphics_engine_up_->get_renderer().set_next_renderer(gui_engine_up_->renderer_p());  
#endif

  init_actors();
  load_data();

  // glfw
  set_glfw_mouse_button_callbacks();
}

void engine::run()
{
  current_time_ = std::chrono::system_clock::now();
  while (!glfwWindowShouldClose(glfw_window_))
  {
    glfwPollEvents();
    process_input();
    update();
    render();
  }
  graphics_engine_up_->wait_idle();
  cleanup();
}

void engine::process_input()
{
}

void engine::update()
{
  is_updating_ = true;

  float dt;
  std::chrono::system_clock::time_point new_time;
  // calc dt
  do {
  new_time = std::chrono::system_clock::now();

  dt = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time_).count();
  } while(dt < 1.0f / MAX_FPS);

  dt = std::min(dt, MAX_DT);

  for (auto& kv : active_actor_map_) {
    const auto id = kv.first;
    auto& actor = kv.second;
    if(actor->get_actor_state() == actor::state::ACTIVE)
      actor->update(dt);
    // check if the actor is dead
    if (actor->get_actor_state() == actor::state::DEAD) {
      dead_actor_ids_.emplace_back(id);
      // erase relevant model comp.
      // TODO dont use hgeActor::id_t but component::id_t
      // if(actor->is_renderable())
        // graphics_engine_up_->remove_renderable_component(id);
    }
  }

  // engine specific update
  update_game(dt);

  // camera
  camera_up_->update(dt);
  light_manager_up_->update(dt);

  current_time_ = new_time;
  is_updating_ = false;

  // activate pending actor
  for (auto& pend : pending_actor_map_) {
    if(pend.second->is_renderable())
      graphics_engine_up_->set_renderable_component(pend.second->get_renderable_component_sp());
    active_actor_map_.emplace(pend.first, std::move(pend.second));
  }
  pending_actor_map_.clear();
  // clear all the dead actors
  for (const auto& id : dead_actor_ids_) {
    if (active_actor_map_[id]->is_renderable())
      graphics_engine_up_->remove_renderable_component_without_owner(
        active_actor_map_[id]->get_renderable_component_sp()->get_render_type(), id);
    active_actor_map_.erase(id);
  }
  dead_actor_ids_.clear();
}

void engine::render()
{
  graphics_engine_up_->render(camera_up_->get_viewer_info());
#ifndef IMGUI_DISABLED
  if (!hnll::graphics::renderer::swap_chain_recreated_){
    gui_engine_up_->begin_imgui();
    update_gui();
    gui_engine_up_->render();
  }
#endif
}

#ifndef IMGUI_DISABLED
void engine::update_gui()
{
  // some general imgui upgrade
  update_game_gui();
  for (auto& kv : active_actor_map_) {
  const id_t& id = kv.first;
  auto& actor = kv.second;
  actor->update_gui();
  }
}
#endif

void engine::init_actors()
{
  // hge actors
  camera_up_ = std::make_shared<default_camera>(*graphics_engine_up_);
  
  // TODO : configure priorities of actors, then update light manager after all light comp
  light_manager_up_ = std::make_shared<point_light_manager>(graphics_engine_up_->get_global_ubo());

}

void engine::load_data()
{
  // load raw mesh data
  load_mesh_models();
  load_actor();
}

// use filenames as the key of the map
// TODO : add models by adding folders or files
void engine::load_mesh_models(const std::string& model_directory)
{
  // auto path = std::string(std::getenv("HNLL_ENGN")) + model_directory;
  auto path = std::string("/home/honolulu/models/primitives");
  for (const auto & file : std::filesystem::directory_iterator(path)) {
    auto filename = std::string(file.path());
    auto length = filename.size() - path.size() - 5;
    auto key = filename.substr(path.size() + 1, length);
    auto mesh_model = hnll::graphics::mesh_model::create_model_from_file(graphics_engine_up_->get_device(), filename);
    mesh_model_map_.emplace(key, std::move(mesh_model));
  }
}

// actors should be created as shared_ptr
void engine::add_actor(const s_ptr<actor>& actor)
{ pending_actor_map_.emplace(actor->get_id(), actor); }

// void engine::add_actor(s_ptr<actor>&& actor)
// { pending_actor_map_.emplace(actor->get_id(), std::move(actor)); }

void engine::remove_actor(id_t id)
{
  pending_actor_map_.erase(id);
  active_actor_map_.erase(id);
  // renderableActorMap_m.erase(id);
}

void engine::load_actor()
{
  auto smooth_vase = actor::create();
  auto smooth_vase_mesh_model = mesh_model_map_["smooth_vase"];
  auto smooth_vase_model_comp = mesh_component::create(smooth_vase, std::move(smooth_vase_mesh_model));
  smooth_vase->set_translation(glm::vec3{-0.5f, 0.5f, 0.f});
  smooth_vase->set_scale(glm::vec3{3.f, 1.5f, 3.f});
  
  // temporary
  hieModelID_ = smooth_vase->get_id();

  auto flat_vase = actor::create();
  auto flat_vase_mesh_model = mesh_model_map_["flat_vase"];
  auto flat_vase_model_comp = mesh_component::create(flat_vase, std::move(flat_vase_mesh_model));
  flat_vase->set_translation(glm::vec3{0.5f, 0.5f, 0.f});
  flat_vase->set_scale(glm::vec3{3.f, 1.5f, 3.f});
  
  auto floor = actor::create();
  auto floor_mesh_comp = mesh_model_map_["quad"];
  auto floor_model_comp = mesh_component::create(floor, std::move(floor_mesh_comp));
  floor->set_translation(glm::vec3{0.f, 0.5f, 0.f});
  floor->set_scale(glm::vec3{3.f, 1.5f, 3.f});
}

void engine::add_point_light(s_ptr<actor>& owner, s_ptr<point_light_component>& light_comp)
{
  // shared by three actor 
  owner->set_renderable_component(light_comp);
  light_manager_up_->add_light_comp(light_comp);
} 

void engine::add_point_light_without_owner(const s_ptr<point_light_component>& light_comp)
{
  // path to the renderer
  graphics_engine_up_->set_renderable_component(light_comp);
  // path to the manager
  light_manager_up_->add_light_comp(light_comp);
}

void engine::remove_point_light_without_owner(component_id id)
{
  graphics_engine_up_->remove_renderable_component_without_owner(render_type::POINT_LIGHT, id);
  light_manager_up_->remove_light_comp(id);
}


void engine::cleanup()
{
  active_actor_map_.clear();
  pending_actor_map_.clear();
  dead_actor_ids_.clear();
  mesh_model_map_.clear();
  hnll::graphics::renderer::cleanup_swap_chain();
}

// glfw
void engine::set_glfw_mouse_button_callbacks()
{
  glfwSetMouseButtonCallback(glfw_window_, glfw_mouse_button_callback);
}

void engine::add_glfw_mouse_button_callback(u_ptr<std::function<void(GLFWwindow* window, int button, int action, int mods)>>&& func)
{
  glfw_mouse_button_callbacks_.emplace_back(std::move(func));
  set_glfw_mouse_button_callbacks();
}

void engine::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  for (const auto& func : glfw_mouse_button_callbacks_)
    func->operator()(window, button, action, mods);
  
#ifndef IMGUI_DISABLED
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
#endif
}

} // namespace game
} // namespace hnll