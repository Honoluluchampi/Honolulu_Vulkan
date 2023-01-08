// hnll
#include <game/engine.hpp>
#include <game/actor.hpp>
#include <game/shading_system.hpp>
#include <game/actors/point_light_manager.hpp>
#include <game/actors/default_camera.hpp>
#include <game/components/mesh_component.hpp>
#include <game/shading_systems/mesh_model_shading_system.hpp>
#include <game/shading_systems/meshlet_model_shading_system.hpp>
#include <game/shading_systems/grid_shading_system.hpp>
#include <game/shading_systems/skinning_mesh_model_shading_system.hpp>
#include <physics/collision_info.hpp>
#include <physics/collision_detector.hpp>
#include <physics/engine.hpp>
#include <graphics/meshlet_model.hpp>
#include <graphics/skinning_mesh_model.hpp>
#include <graphics/frame_anim_mesh_model.hpp>
#include <graphics/frame_anim_meshlet_model.hpp>

// lib
#include <imgui.h>

// std
#include <filesystem>
#include <iostream>

namespace hnll::game {

constexpr float MAX_FPS = 60.0f;
constexpr float MAX_DT = 0.05f;

// static members
u_ptr<graphics_engine>  engine::graphics_engine_{};
actor_map               engine::active_actor_map_{};
actor_map               engine::pending_actor_map_{};
std::vector<actor_id>   engine::dead_actor_ids_{};
mesh_model_map          engine::mesh_model_map_;
meshlet_model_map       engine::meshlet_model_map_;
skinning_mesh_model_map engine::skinning_mesh_model_map_;
frame_anim_mesh_model_map engine::frame_anim_mesh_model_map_;
frame_anim_meshlet_model_map engine::frame_anim_meshlet_model_map_;

// glfw
GLFWwindow* engine::glfw_window_;
std::vector<u_ptr<std::function<void(GLFWwindow*, int, int, int)>>> engine::glfw_mouse_button_callbacks_{};

engine::engine(const char* window_name)
{
  graphics_engine_ = std::make_unique<graphics_engine>(window_name);

  set_glfw_window();

#ifndef IMGUI_DISABLED
  gui_engine_ = std::make_unique<hnll::gui::engine>
    (graphics_engine_->get_window_r(), graphics_engine_->get_device_r());
  // configure dependency between renderers
  graphics_engine_->get_renderer_r().set_next_renderer(gui_engine_->renderer_p());
#endif

  setup_shading_systems();
  init_actors();
  load_data();

  // glfw
  set_glfw_mouse_button_callbacks();
}

engine::~engine() = default;

void engine::run()
{
  current_time_ = std::chrono::system_clock::now();
  while (!glfwWindowShouldClose(glfw_window_))
  {
    glfwPollEvents();
    process_input();
    update();
    // TODO : implement as physics engine
    re_update_actors();

    render();
  }
  graphics_engine_->wait_idle();
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
//  do {
  new_time = std::chrono::system_clock::now();

  dt = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time_).count();
//  } while(dt < 1.0f / MAX_FPS);

  dt = std::min(dt, MAX_DT);

  for (auto& kv : active_actor_map_) {
    const auto id = kv.first;
    auto& actor = kv.second;
    if(actor->get_actor_state() == actor::state::ACTIVE)
      actor->update(dt);
    // check if the actor is dead
    if (actor->get_actor_state() == actor::state::DEAD) {
      dead_actor_ids_.emplace_back(id);
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
      graphics_engine_->add_renderable_component(pend.second->get_renderable_component_r());
    active_actor_map_.emplace(pend.first, std::move(pend.second));
  }
  pending_actor_map_.clear();
  // clear all the dead actors
  for (const auto& id : dead_actor_ids_) {
    if (active_actor_map_[id]->is_renderable())
      graphics_engine_->remove_renderable_component(active_actor_map_[id]->get_renderable_component_r());
    active_actor_map_.erase(id);
  }
  dead_actor_ids_.clear();
}


// physics
void engine::re_update_actors()
{
  physics_engine_->re_update();
}

void engine::render()
{
  viewer_info_  = camera_up_->get_viewer_info();
  graphics_engine_->render(viewer_info_, frustum_info_);

  #ifndef IMGUI_DISABLED
  if (!hnll::graphics::renderer::swap_chain_recreated_){
    gui_engine_->begin_imgui();
    update_gui();
    gui_engine_->render();
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

void engine::setup_shading_systems()
{
  auto grid_shader = game::grid_shading_system::create(get_graphics_device());
  add_shading_system(std::move(grid_shader));
}

void engine::init_actors()
{
  // hge actors
  camera_up_ = std::make_shared<default_camera>(*graphics_engine_);
  
  // TODO : configure priorities of actors, then update light manager after all light comp
  light_manager_up_ = std::make_shared<point_light_manager>(graphics_engine_->get_global_ubo_r());
}

void engine::load_data()
{
  // load raw mesh data
  // load_models();
  // load_actor();
}

void engine::load_models()
{
  for (const auto& path : utils::loading_directories) {
    for (const auto& file : std::filesystem::directory_iterator(path)) {
      auto filename = std::string(file.path());
      auto extension = std::string(file.path().extension());
      auto length = filename.size() - path.size();
      auto key = filename.substr(path.size() + 1, length);

      if (extension == ".obj" && graphics_engine_->check_shading_system_exists(utils::shading_type::MESH)) {
        // mesh model
        auto mesh_model = hnll::graphics::mesh_model::create_from_file(get_graphics_device(), key);
        mesh_model_map_.emplace(key, std::move(mesh_model));
      }
      if (extension == ".obj" && graphics_engine_->check_shading_system_exists(utils::shading_type::MESHLET)) {
        // meshlet model
        auto meshlet_model = hnll::graphics::meshlet_model::create_from_file(get_graphics_device(), key);
        std::cout << key << " :" << std::endl << "\tmeshlet count : " << meshlet_model->get_meshlets_count() << std::endl;
        meshlet_model_map_.emplace(key, std::move(meshlet_model));
      }

      if (extension == ".glb") {
        auto skinning_model = hnll::graphics::skinning_mesh_model::create_from_gltf(filename, get_graphics_device());
        skinning_mesh_model_map_.emplace(key, std::move(skinning_model));
      }
    }
  }
}

void engine::load_model(const std::string &model_name, utils::shading_type type)
{
  auto full_path = utils::get_full_path(model_name);
  std::filesystem::path path = { full_path };

  switch (type) {
    case utils::shading_type::MESH : {
      if (path.extension().string() == ".obj") {
        check_and_add_shading_system<mesh_model_shading_system>(type);
        auto model = graphics::mesh_model::create_from_file(get_graphics_device(), path.filename().string());
        mesh_model_map_.emplace(path.filename().string(), std::move(model));
      }
      else
        std::cerr << "extension " << path.extension().string() << " is not supported for shading_type::MESH." << std::endl;
      break;
    }
    case utils::shading_type::MESHLET : {
      if (path.extension().string() == ".obj") {
        check_and_add_shading_system<meshlet_model_shading_system>(type);
        auto model = graphics::meshlet_model::create_from_file(get_graphics_device(), path.filename().string());
        meshlet_model_map_.emplace(path.filename().string(), std::move(model));
      }
      else
        std::cerr << "extension " << path.extension().string() << " is not supported for shading_type::MESHLET." << std::endl;
      break;
    }
    case utils::shading_type::SKINNING_MESH : {
      if (path.extension().string() == ".glb") {
        check_and_add_shading_system<skinning_mesh_model_shading_system>(type);
        auto model = graphics::skinning_mesh_model::create_from_gltf(path.string(), get_graphics_device());
        skinning_mesh_model_map_.emplace(path.filename().string(), std::move(model));
      }
      else
        std::cerr << "extension " << path.extension().string() << " is not supported for shading_type::SKINNING_MESH." << std::endl;
      break;
    }
    // TODO : add frame_anim_mesh_model and frame_anim_meshlet_model
  }
}

// actors should be created as shared_ptr
void engine::add_actor(const s_ptr<actor>& actor)
{ pending_actor_map_.emplace(actor->get_id(), actor); }

void engine::remove_actor(id_t id)
{
  pending_actor_map_.erase(id);
  active_actor_map_.erase(id);
}

void engine::add_shading_system(u_ptr<hnll::game::shading_system> &&shading_system)
{
  graphics_engine_->add_shading_system(std::move(shading_system));
}

void engine::load_actor()
{
  auto smooth_vase = actor::create();
  auto smooth_vase_model_comp = mesh_component::create(smooth_vase, "smooth_sphere.obj");
  smooth_vase->set_translation(glm::vec3{0.f, 0.f, 3.f});

  auto flat_vase = actor::create();
  auto flat_vase_model_comp = mesh_component::create(flat_vase, "light_bunny.obj");
  flat_vase->set_translation(glm::vec3{0.5f, 0.5f, 0.f});
  flat_vase->set_scale(glm::vec3{3.f, 1.5f, 3.f});
  
  auto floor = actor::create();
  auto floor_model_comp = mesh_component::create(floor, "plane.obj");
  floor->set_translation(glm::vec3{0.f, 0.5f, 0.f});
  floor->set_scale(glm::vec3{3.f, 1.5f, 3.f});

  float light_intensity = 4.f;
  std::vector<glm::vec3> positions;
  float position_radius = 4.f;
  for (int i = 0; i < 6; i++) {
    positions.push_back({position_radius * std::sin(M_PI/3.f * i), -2.f, position_radius * std::cos(M_PI/3.f * i)});
  }
  positions.push_back({0.f, position_radius, 0.f});
  positions.push_back({0.f, -position_radius, 0.f});

  for (const auto& position : positions) {
    auto light = hnll::game::actor::create();
    auto light_component = hnll::game::point_light_component::create(light, light_intensity, 0.f);
    light_component->set_color(glm::vec3(0.f, 1.f, 0.3f));
    light_component->set_radius(0.5f);
    add_point_light(light, light_component);
    light->set_translation(position);
  }
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
  graphics_engine_->add_renderable_component(*light_comp);
  // path to the manager
  light_manager_up_->add_light_comp(light_comp);
}

void engine::remove_point_light_without_owner(component_id id)
{
  graphics_engine_->remove_renderable_component(utils::shading_type::POINT_LIGHT, id);
  light_manager_up_->remove_light_comp(id);
}

void engine::cleanup()
{
  active_actor_map_.clear();
  pending_actor_map_.clear();
  dead_actor_ids_.clear();
  mesh_model_map_.clear();
  meshlet_model_map_.clear();
  skinning_mesh_model_map_.clear();
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

void engine::set_frustum_info(utils::frustum_info &&_frustum_info)
{
  frustum_info_ = std::move(_frustum_info);
}

graphics::meshlet_model& engine::get_meshlet_model(const std::string& model_name)
{
  if (skinning_mesh_model_map_.find(model_name) == skinning_mesh_model_map_.end()) {
    load_model(model_name, utils::shading_type::MESHLET);
  }
  return *meshlet_model_map_[model_name];
}

graphics::skinning_mesh_model& engine::get_skinning_mesh_model(const std::string& model_name)
{
  if (skinning_mesh_model_map_.find(model_name) == skinning_mesh_model_map_.end()) {
    load_model(model_name, utils::shading_type::SKINNING_MESH);
  }
  return *skinning_mesh_model_map_[model_name]; }

graphics::frame_anim_meshlet_model& engine::get_frame_anim_meshlet_model(const std::string& model_name)
{ return *frame_anim_meshlet_model_map_[model_name]; }

//actor& engine::get_active_actor(actor_id id)
//{ return *active_actor_map_[id]; }


} // namespace hnll::game
