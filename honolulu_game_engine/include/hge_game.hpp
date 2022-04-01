#pragma once

//  hve
#include <hve.hpp>
#include <hge_actor.hpp>

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
  ~HgeGame(){}
  // delete copy ctor
  HgeGame(const HgeGame &) = delete;
  HgeGame& operator=(const HgeGame &) = delete;

  bool initialize();
  void runLoop();

  void addActor(const class HveActor& actor);
  std::unique_ptr<Hve> upHve_m;
  void removeActor(const class HveActor& actor);

private:
  inline void setGLFWwindow() { glfwWindow_m = upHve_m->passGLFWwindow() ; }
  void processInput();
  void update();
  void generateOutput();

  void loadData();
  void unLoadData();
  // load all models in modleDir
  // use filenames as the key of the map
  void loadHveModels(const std::string& modelDir = "/models");

  GLFWwindow* glfwWindow_m;
  std::vector<std::unique_ptr<HgeActor>> upActiveActors_m;
  std::vector<std::unique_ptr<HgeActor>> upPendingActors_m;

  // map of models
  // models would be shared by some actors
  // wanna make it boost::intrusive_ptr 
  std::unordered_map<std::string, std::shared_ptr<HveModel>> spHveModels_m;

  bool isUpdating_m = false; // for update
  bool isRunning_m = false; // for run loop

  // create in a heap
};

} // namespace hnll