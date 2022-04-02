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
  upHve_m->render(dt, spModelComps_m);

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
  createGameObjectTemp();
  //upHve_m->createGameObjects(spModelComps_m);
}

// use filenames as the key of the map
void HgeGame::loadHveModels(const std::string& modelDir)
{
  auto path = std::string(std::filesystem::current_path()) + modelDir;
  for (const auto & file : std::filesystem::directory_iterator(path)) {
    auto filename = std::string(file.path());
    auto length = filename.size() - path.size() - 5;
    auto key = filename.substr(path.size() + 1, length);
    auto modelComp = HveModel::createModelFromFile(upHve_m->hveDevice(), filename);
    spModelComps_m.emplace(key, std::make_shared<ModelComponent>(modelComp));
  }
}

void HgeGame::createGameObjectTemp()
{
  auto& smoothVase = spModelComps_m["smooth_vase"];
  smoothVase->transform_m.translation_m = {-0.5f, 0.5f, 0.f};
  smoothVase->transform_m.scale_m = {3.f, 1.5f, 3.f};
  
  auto& flatVaseModel = spModelComps_m["flat_vase"];
  flatVaseModel->transform_m.translation_m = {0.5f, 0.5f, 0.f};
  flatVaseModel->transform_m.scale_m = glm::vec3{3.f, 1.5f, 3.f};

  auto& floor = spModelComps_m["quad"];
  floor->transform_m.translation_m = {0.f, 0.5f, 0.f};
  floor->transform_m.scale_m = glm::vec3{3.f, 1.5f, 3.f};

  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f},
      {.1f, .1f, 1.f},
      {.1f, 1.f, .1f},
      {1.f, 1.f, .1f},
      {.1f, 1.f, 1.f},
      {1.f, 1.f, 1.f} 
  };

  for (int i = 0; i < lightColors.size(); i++) {
    auto pointLight = HveGameObject::makePointLight(0.2f);
    pointLight.color_m = lightColors[i];
    auto lightRotation = glm::rotate(
        glm::mat4(1),
        (i * glm::two_pi<float>()) / lightColors.size(),
        {0.f, -1.0f, 0.f}); // axiz
    pointLight.transform_m.translation_m = glm::vec3(lightRotation * glm::vec4(-1.f, -1.f, -1.f, 1.f));
    //gameObjects_m.emplace(pointLight.getId(), std::move(pointLight));
  }
}

} // namespace hnll