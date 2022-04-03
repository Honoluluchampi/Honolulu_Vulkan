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

  for (auto& actor : activeActorMap_m) {
    actor.second->update(dt);
    // check if the actor is dead
    if (actor.second->getActorState() == HgeActor::state::DEAD) {
      deadActorMap_m.emplace(actor.first, std::move(actor.second));
      activeActorMap_m.erase(actor.first);
      // erase relevant model comp.
      modelCompMap_m.erase(actor.first);
    }
  }

  // camera update
  upHve_m->update(dt);

  currentTime_m = newTime;
  isUpdating_m = false;

  // activate pending actor
  for (auto& pend : pendingActorMap_m) {
    activeActorMap_m.emplace(pend.first, std::move(pend.second));
  }
  pendingActorMap_m.clear();
  // clear all the dead actors
  deadActorMap_m.clear();

  upHve_m->render(dt, modelCompMap_m);
}

void HgeGame::generateOutput()
{
}

void HgeGame::loadData()
{
  // load raw data
  loadHveModels();
  // share above data with vulkan engine
  createActor();
}

// use filenames as the key of the map
void HgeGame::loadHveModels(const std::string& modelDir)
{
  auto path = std::string(std::filesystem::current_path()) + modelDir;
  for (const auto & file : std::filesystem::directory_iterator(path)) {
    auto filename = std::string(file.path());
    auto length = filename.size() - path.size() - 5;
    auto key = filename.substr(path.size() + 1, length);
    auto hveModel = HveModel::createModelFromFile(upHve_m->hveDevice(), filename);
    hveModelMap_m.emplace(key, std::move(hveModel));
  }
}

// actors should be created as unique_ptr
void HgeGame::addActor(std::unique_ptr<HgeActor>& actor)
{ pendingActorMap_m.emplace(actor->getId(), std::move(actor)); }

void HgeGame::addActor(std::unique_ptr<HgeActor>&& actor)
{ pendingActorMap_m.emplace(actor->getId(), std::move(actor)); }

void HgeGame::removeActor(id_t id)
{
  pendingActorMap_m.erase(id);
  activeActorMap_m.erase(id);
  // renderableActorMap_m.erase(id);
}

void HgeGame::createActor()
{
  auto smoothVase = std::make_unique<HgeActor>(HgeActor::createActor());
  auto& smoothVaseHveModel = hveModelMap_m["smooth_vase"];
  auto smoothVaseModelComp = std::make_shared<ModelComponent>(smoothVase->getId(), smoothVaseHveModel);
  smoothVase->addSharedComponent(smoothVaseModelComp);
  smoothVaseModelComp->setTranslation(glm::vec3{-0.5f, 0.5f, 0.f});
  smoothVaseModelComp->setScale(glm::vec3{3.f, 1.5f, 3.f});
  
  modelCompMap_m.emplace(smoothVase->getId(), std::move(smoothVaseModelComp));
  addActor(std::move(smoothVase));
  // auto& flatVaseModel = spModelComps_m["flat_vase"];
  // flatVaseModel->setTranslation(glm::vec3{0.5f, 0.5f, 0.f});
  // flatVaseModel->setScale(glm::vec3{3.f, 1.5f, 3.f});

  // auto& floor = spModelComps_m["quad"];
  // floor->setTranslation(glm::vec3{0.f, 0.5f, 0.f});
  // floor->setScale(glm::vec3{3.f, 1.5f, 3.f});

  // std::vector<glm::vec3> lightColors{
  //     {1.f, .1f, .1f},
  //     {.1f, .1f, 1.f},
  //     {.1f, 1.f, .1f},
  //     {1.f, 1.f, .1f},
  //     {.1f, 1.f, 1.f},
  //     {1.f, 1.f, 1.f} 
  // };

  // for (int i = 0; i < lightColors.size(); i++) {
  //   auto pointLight = HveGameObject::makePointLight(0.2f);
  //   pointLight.color_m = lightColors[i];
  //   auto lightRotation = glm::rotate(
  //       glm::mat4(1),
  //       (i * glm::two_pi<float>()) / lightColors.size(),
  //       {0.f, -1.0f, 0.f}); // axiz
  //   pointLight.transform_m.translation_m = glm::vec3(lightRotation * glm::vec4(-1.f, -1.f, -1.f, 1.f));
  //   //gameObjects_m.emplace(pointLight.getId(), std::move(pointLight));
  // }
}

} // namespace hnll