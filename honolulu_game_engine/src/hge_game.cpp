#include <hge_game.hpp>

// std
#include <filesystem>
#include <iostream>

namespace hnll {

constexpr float MAX_DT = 0.05f;

HgeGame::HgeGame(const char* windowName) : upHve_m(std::make_unique<Hve>(windowName))
{
  setGLFWwindow();
  loadData();
}

void HgeGame::run()
{
  currentTime_m = std::chrono::high_resolution_clock::now();
  while (!glfwWindowShouldClose(glfwWindow_m))
  {
    glfwPollEvents();
    processInput();
    update();
    generateOutput();
  }

  upHve_m->waitIdle();
}

void HgeGame::processInput()
{

}

void HgeGame::update()
{
  isUpdating_m = true;
  // calc dt
  auto newTime = std::chrono::high_resolution_clock::now();
  float dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime_m).count();
  dt = std::min(dt, MAX_DT);

  for (auto& actor : upActiveActors_m) {
    // actor->update(dt);
  }

  upHve_m->update(dt);
  upHve_m->render(dt);

  currentTime_m = newTime;
  isUpdating_m = false;
}

void HgeGame::generateOutput()
{
}

void HgeGame::loadData()
{
  // load raw data
  loadHveModels();
  // share above data with vulkan engine
  upHve_m->createGameObjects(spHveModels_m);
}

// use filenames as the key of the map
void HgeGame::loadHveModels(const std::string& modelDir)
{
  auto path = std::string(std::filesystem::current_path()) + modelDir;
  for (const auto & file : std::filesystem::directory_iterator(path)) {
    auto filename = std::string(file.path());
    auto length = filename.size() - path.size() - 5;
    auto key = filename.substr(path.size() + 1, length);
    spHveModels_m.emplace(key, HveModel::createModelFromFile(upHve_m->hveDevice(), filename));
  }
}

} // namespace hnll