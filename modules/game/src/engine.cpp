// hnll
#include <game/engine.hpp>

// lib
#include <imgui.h>

// std
#include <filesystem>
#include <iostream>
#include <typeinfo>

namespace hnll {

constexpr float MAX_FPS = 30.0f;
constexpr float MAX_DT = 0.05f;

// static members
actor::map game::pending_actor_map_;

// glfw
GLFWwindow* game::glfwWindow_m;
std::vector<u_ptr<std::function<void(GLFWwindow*, int, int, int)>>> game::glfwMouseButtonCallbacks_{};

// x11
// Display* game::display_ = XOpenDisplay(NULL);

game::game(const char* windowName) : graphics_engine_up_(std::make_unique<engine>(windowName))
{
  set_glfw_window(); // ?

#ifndef IMGUI_DISABLED
  gui_engine_up_ = std::make_unique<Hie>
    (graphics_engine_up_->get_window(), graphics_engine_up_->get_graphics_device());
  // configure dependency between renderers
  graphics_engine_up_->get_renderer().set_next_renderer(gui_engine_up_->renderer_p_());  
#endif

  init_actors();
  load_data();

  // glfw
  set_glfw_mouse_button_callbacks();
}

game::~game()
{
  // cleanup in game::cleanup();
  // renderer::cleanup_swap_chain();
}

void game::run()
{
  current_time_ = std::chrono::system_clock::now();
  while (!glfwWindowShouldClose(glfwWindow_m))
  {
    glfwPollEvents();
    process_input();
    update();
    render();
  }
  graphics_engine_up_->wait_idle();
  cleanup();
}

void game::process_input()
{
}

void game::update()
{
  is_updating_ = true;

  float dt;
  std::chrono::system_clock::time_point newTime;
  // calc dt
  do {
  newTime = std::chrono::system_clock::now();

  dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - current_time_).count();
  } while(dt < 1.0f / MAX_FPS);

  dt = std::min(dt, MAX_DT);

  for (auto& kv : active_actor_map_) {
    const id_t& id = kv.first;
    auto& actor = kv.second;
    actor->update(dt);
    // check if the actor is dead
    if (actor->get_actor_state() == actor::state::DEAD) {
      dead_actor_map_.emplace(id, std::move(actor));
      active_actor_map_.erase(id);
      // erase relevant model comp.
      // TODO dont use hgeActor::id_t but component::id_t
      // if(actor->is_renderable())
        // graphics_engine_up_->remove_renderable_component(id);
    }
  }

  // game specific update
  update_game(dt);

  // camera
  camera_up_->update(dt);
  upLightManager_->update(dt);

  current_time_ = newTime;
  is_updating_ = false;

  // activate pending actor
  for (auto& pend : pending_actor_map_) {
    if(pend.second->is_renderable())
      graphics_engine_up_->set_renderable_component(pend.second->get_renderable_component());
    active_actor_map_.emplace(pend.first, std::move(pend.second));
  }
  pending_actor_map_.clear();
  // clear all the dead actors
  dead_actor_map_.clear();

  // TODO : delete gui demo
  // if (renderableComponentID_m != -1)
  //   HgeRenderableComponent& comp = dynamic_cast<HgeRenderableComponent&>(active_actor_map_[hieModelID_]->get_renderable_component());
  // gui_engine_up_->update(comp.get_transform().translation);
}

void game::render()
{

  graphics_engine_up_->render(*(camera_up_->viewer_component()));
#ifndef IMGUI_DISABLED
  if (!renderer::swap_chain_recreated_){
    gui_engine_up_->begin_imgui();
    update_gui();
    gui_engine_up_->render();
  }
#endif
}

#ifndef IMGUI_DISABLED
void game::update_gui()
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

void game::init_actors()
{
  // hge actors
  camera_up_ = std::make_shared<default_camera>(*graphics_engine_up_);
  
  // TODO : configure priorities of actors, then update light manager after all light comp
  upLightManager_ = std::make_shared<point_light_manager>(graphics_engine_up_->get_global_ubo());

}

void game::load_data()
{
  // load raw data
  load_mesh_models();
  // temporary
  // load_actor();
}

// use filenames as the key of the map
// TODO : add models by adding folders or files
void game::load_mesh_models(const std::string& modelDir)
{
  // auto path = std::string(std::getenv("HNLL_ENGN")) + modelDir;
  auto path = std::string("/home/honolulu/models/primitives");
  for (const auto & file : std::filesystem::directory_iterator(path)) {
    auto filename = std::string(file.path());
    auto length = filename.size() - path.size() - 5;
    auto key = filename.substr(path.size() + 1, length);
    auto hveModel = mesh_model::create_model_from_file(graphics_engine_up_->get_graphics_device(), filename);
    mesh_model_map_.emplace(key, std::move(hveModel));
  }
}

// actors should be created as shared_ptr
void game::add_actor(const s_ptr<actor>& actor)
{ pending_actor_map_.emplace(actor->get_id(), actor); }

// void game::add_actor(s_ptr<actor>&& actor)
// { pending_actor_map_.emplace(actor->get_id(), std::move(actor)); }

void game::remove_actor(id_t id)
{
  pending_actor_map_.erase(id);
  active_actor_map_.erase(id);
  // renderableActorMap_m.erase(id);
}

void game::load_actor()
{
  auto smoothVase = create_actor();
  auto& smoothVaseHveModel = mesh_model_map_["smooth_vase"];
  auto smoothVaseModelComp = std::make_shared<mesh_component>(smoothVase->get_id(), smoothVaseHveModel);
  smoothVase->set_renderable_component(smoothVaseModelComp);
  smoothVaseModelComp->set_translation(glm::vec3{-0.5f, 0.5f, 0.f});
  smoothVaseModelComp->set_scale(glm::vec3{3.f, 1.5f, 3.f});
  
  // temporary
  hieModelID_ = smoothVase->get_id();

  auto flatVase = create_actor();
  auto& flatVaseHveModel = mesh_model_map_["flat_vase"];
  auto flatVaseModelComp = std::make_shared<mesh_component>(flatVase->get_id(), flatVaseHveModel);
  flatVase->set_renderable_component(flatVaseModelComp);
  flatVaseModelComp->set_translation(glm::vec3{0.5f, 0.5f, 0.f});
  flatVaseModelComp->set_scale(glm::vec3{3.f, 1.5f, 3.f});
  
  auto floor = create_actor();
  auto& floorHveModel = mesh_model_map_["quad"];
  auto floorModelComp = std::make_shared<mesh_component>(floor->get_id(), floorHveModel);
  floor->set_renderable_component(floorModelComp);
  floorModelComp->set_translation(glm::vec3{0.f, 0.5f, 0.f});
  floorModelComp->set_scale(glm::vec3{3.f, 1.5f, 3.f});

  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f},
      {.1f, .1f, 1.f},
      {.1f, 1.f, .1f},
      {1.f, 1.f, .1f},
      {.1f, 1.f, 1.f},
      {1.f, 1.f, 1.f} 
  };

  for (int i = 0; i < lightColors.size(); i++) {
    auto lightActor = create_actor();
    auto lightComp = point_light_component::create_point_light(lightActor->get_id(), 1.0f, 0.f, lightColors[i]);
    auto lightRotation = glm::rotate(
        glm::mat4(1),
        (i * glm::two_pi<float>()) / lightColors.size(),
        {0.f, -1.0f, 0.f}); // axiz
    lightComp->set_translation(glm::vec3(lightRotation * glm::vec4(-1.f, -1.f, -1.f, 1.f)));
    add_point_light(lightActor, lightComp);
  }
}

void game::add_point_light(s_ptr<actor>& owner, s_ptr<point_light_component>& lightComp)
{
  // shared by three actor 
  owner->set_renderable_component(lightComp);
  upLightManager_->add_light_comp(lightComp);
} 

void game::add_point_light_without_owner(s_ptr<point_light_component>& lightComp)
{
  // path to the renderer
  graphics_engine_up_->set_renderable_component(lightComp);
  // path to the manager
  upLightManager_->add_light_comp(lightComp);
}

void game::remove_point_light_without_owner(component::id id)
{
  graphics_engine_up_->remove_renderable_component_without_owner(render_type::POINT_LIGHT, id);
  upLightManager_->remove_light_comp(id);
}


void game::cleanup()
{
  active_actor_map_.clear();
  pending_actor_map_.clear();
  dead_actor_map_.clear();
  mesh_model_map_.clear();
  renderer::cleanup_swap_chain();
}

// glfw
void game::set_glfw_mouse_button_callbacks()
{
  glfwSetMouseButtonCallback(glfwWindow_m, glfw_mouse_button_callback);
}

void game::add_glfw_mouse_button_callback(u_ptr<std::function<void(GLFWwindow* window, int button, int action, int mods)>>&& func)
{
  glfwMouseButtonCallbacks_.emplace_back(std::move(func));
  set_glfw_mouse_button_callbacks();
}

void game::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  for (const auto& func : glfwMouseButtonCallbacks_)
    func->operator()(window, button, action, mods);
  
#ifndef IMGUI_DISABLED
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
#endif
}

} // namespace hnll