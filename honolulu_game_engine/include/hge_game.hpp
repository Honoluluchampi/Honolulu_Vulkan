#pragma once

//  hve
#include <hve.hpp>
#include <hge_actor.hpp>
#include <hge_components/model_component.hpp>
#include <hge_actors/hge_default_camera.hpp>
#include <hge_actors/hge_point_light_manager.hpp>

// hie
#include <hie.hpp>

// lib
#include <GLFW/glfw3.h>

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

  void addActor(u_ptr<HgeActor>& actor);
  void addActor(u_ptr<HgeActor>&& actor);
  void addPointLight(u_ptr<HgeActor>& owner, s_ptr<PointLightComponent>& lightComp);
  void removeActor(id_t id);

protected:
  GLFWwindow* glfwWindow_m;
  // hge actors
  u_ptr<HgeCamera> upCamera_m;
  u_ptr<HgePointLightManager> upLightManager_;

private:
  inline void setGLFWwindow() { glfwWindow_m = upHve_m->passGLFWwindow() ; }
  void cleanup();
  void processInput();
  void update();
  void render();

  // init 
  void initHgeActors();
  void loadData();
  virtual void createActor();

  void unLoadData();
  // load all models in modleDir
  // use filenames as the key of the map
  void loadHveModels(const std::string& modelDir = "/models");


  HgeActor::map activeActorMap_m;
  HgeActor::map pendingActorMap_m;
  HgeActor::map deadActorMap_m;

  u_ptr<Hve> upHve_m;

#ifndef __IMGUI_DISABLED
  u_ptr<Hie> upHie_m;
#endif

  // map of modelcomponents
  // shared by game and some actors
  // wanna make it boost::intrusive_ptr 
  // map of HveModel
  // shared by game and some modelComponents
  // pool all models which would be necessary
  HveModel::map hveModelMap_m;

  bool isUpdating_m = false; // for update
  bool isRunning_m = false; // for run loop

  std::chrono::_V2::system_clock::time_point currentTime_m;

  id_t hieModelID_;
};

} // namespace hnll