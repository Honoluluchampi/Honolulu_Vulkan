#include <hge_game.hpp>

// std
#include <filesystem>
#include <iostream>

namespace hnll {

HgeGame::HgeGame(const char* windowName) : upHve_m(std::make_unique<Hve>(windowName))
{
  setGLFWwindow();
  loadData();
}

void HgeGame::runLoop()
{
  while (!glfwWindowShouldClose(glfwWindow_m))
  {
    glfwPollEvents();
    upHve_m->render();
  }

  upHve_m->waitIdle();
}

void HgeGame::processInput()
{

}

void HgeGame::update()
{

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