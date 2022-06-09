#pragma once

//  hve
#include <hve.hpp>
#include <hge_actor.hpp>
#include <hge_components/mesh_component.hpp>
#include <hge_actors/hge_default_camera.hpp>
#include <hge_actors/hge_point_light_manager.hpp>

// hie
#include <hie.hpp>

// lib
#include <GLFW/glfw3.h>
#include <X11/extensions/XTest.h>

//std
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

namespace hnll {

class HgeGame
{
public:
  HgeGame(const char* windowName = "honolulu engine");
  ~HgeGame();
  // delete copy ctor
  HgeGame(const HgeGame &) = delete;
  HgeGame& operator=(const HgeGame &) = delete;

  bool initialize();
  void run();

  void addActor(const s_ptr<HgeActor>& actor);
  // void addActor(s_ptr<HgeActor>&& actor);
  void removeActor(id_t id);

  // factory funcs
  // takes hgeActor derived class as template argument
  template<class ActorClass = HgeActor, class... Args>
  static s_ptr<ActorClass> createActor(Args... args)
  {
    auto actor = std::make_shared<ActorClass>(args...);
    // create s_ptr of actor perform as HgeActor
    std::shared_ptr<HgeActor> ptr4actorMap = actor;
    // register it to the actor map
    pendingActorMap_m.emplace(ptr4actorMap->getId(), ptr4actorMap);
    return actor;
  }

  // takes s_ptr<HgeRenderableComponent>
  template <class S>
  void addRenderableComponent(S&& comp)
  { upHve_m->addRenderableComponent(std::forward<S>(comp)); }
  template <class S>
  void replaceRenderableComponent(S&& comp)
  { upHve_m->replaceRenderableComponent(std::forward<S>(comp)); }
  void removeRenderableComponent(RenderType type, HgeComponent::compId id)
  { upHve_m->removeRenderableComponentWithoutOwner(type, id); }

  void addPointLight(s_ptr<HgeActor>& owner, s_ptr<PointLightComponent>& lightComp);
  // TODO : delete this func
  void addPointLightWithoutOwner(s_ptr<PointLightComponent>& lightComp);
  void removePointLightWithoutOwner(HgeComponent::compId id);

  void setCameraTransform(const Transform& transform)
  { upCamera_m->getTransform() = transform; }

  // getter
  Hve& hve() { return *upHve_m; }
  HveDevice& hveDevice() { return upHve_m->hveDevice(); }
  s_ptr<HveModel> getHveModel(std::string modelName) 
  { return hveModelMap_m[modelName]; }

#ifndef __IMGUI_DISABLED
  u_ptr<Hie>& hie() { return upHie_m; }
#endif

  // glfw
  // move u_ptr<func> before add
  static void addGlfwMouseButtonCallback(u_ptr<std::function<void(GLFWwindow*, int, int, int)>>&& func);

  // X11
  static Display* x11Display() { return display_; }
protected:
  // TODO : remove static
  static GLFWwindow* glfwWindow_m;
  // hge actors
  s_ptr<HgeCamera> upCamera_m;
  s_ptr<HgePointLightManager> upLightManager_;

private:
  inline void setGLFWwindow() { glfwWindow_m = upHve_m->passGLFWwindow() ; }
  void cleanup();
  void processInput();
  void update();
  // game spacific update
  virtual void updateGame(float dt){}
  void render();

#ifndef __IMGUI_DISABLED
  void updateImgui();
  virtual void updateGameImgui(){}
#endif

  // init 
  void initHgeActors();
  void loadData();
  virtual void loadActor();

  void unLoadData();
  // load all models in modleDir
  // use filenames as the key of the map
  void loadHveModels(const std::string& modelDir = "/models");

  // glfw
  static void setGlfwMouseButtonCallbacks();
  static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

  HgeActor::map activeActorMap_m;
  static HgeActor::map pendingActorMap_m;
  HgeActor::map deadActorMap_m;

  u_ptr<Hve> upHve_m;

#ifndef __IMGUI_DISABLED
  u_ptr<Hie> upHie_m;
#endif

  // map of HveModel
  // shared by game and some modelComponents
  // pool all models which could be necessary
  HveModel::map hveModelMap_m;

  bool isUpdating_m = false; // for update
  bool isRunning_m = false; // for run loop

  std::chrono::_V2::system_clock::time_point currentTime_m;

  id_t hieModelID_;

  // glfw
  static std::vector<u_ptr<std::function<void(GLFWwindow*, int, int, int)>>> 
    glfwMouseButtonCallbacks_;

  // X11
  static Display* display_;
};

} // namespace hnll