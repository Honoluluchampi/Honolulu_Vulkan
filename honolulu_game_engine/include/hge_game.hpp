#pragma once

//  hve
#include <hve.hpp>
#include <hge_actor.hpp>
#include <hge_components/model_component.hpp>
#include <hge_default_camera.hpp>

// hie
#include <hie.hpp>

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

  void addActor(std::unique_ptr<HgeActor>& actor);
  void addActor(std::unique_ptr<HgeActor>&& actor);
  void removeActor(id_t id);

private:
  inline void setGLFWwindow() { glfwWindow_m = upHve_m->passGLFWwindow() ; }
  void cleanup();
  void processInput();
  void update();
  void render();

  void createActor();

  void loadData();
  void unLoadData();
  // load all models in modleDir
  // use filenames as the key of the map
  void loadHveModels(const std::string& modelDir = "/models");

  GLFWwindow* glfwWindow_m;
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
  
  u_ptr<HgeCamera> upCamera_m;

  id_t hieModelID_;
};

} // namespace hnll