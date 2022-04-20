#include <hge_game.hpp>
#include <hge_default_camera.hpp>

// std
#include <filesystem>
#include <iostream>

namespace hnll {

constexpr float MAX_DT = 0.05f;

HgeGame::HgeGame(const char* windowName) : upHve_m(std::make_unique<Hve>(windowName))
{
  setGLFWwindow(); // ?
  upHie_m = std::make_unique<Hie>
    (upHve_m->hveDevice(), upHve_m->hveSwapChain(), upHve_m->passGLFWwindow());
  // camera creation
  upCamera_m = std::make_unique<HgeCamera>(*upHve_m);
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
    render();
  }
  upHve_m->waitIdle();
  cleanup();
}

void HgeGame::processInput()
{

}

// TODO : recreate swap chain
void HgeGame::update()
{
  isUpdating_m = true;
  // calc dt
  auto newTime = std::chrono::high_resolution_clock::now();
  float dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime_m).count();
  dt = std::min(dt, MAX_DT);

  for (auto& kv : activeActorMap_m) {
    const id_t& id = kv.first;
    auto& actor = kv.second;
    actor->update(dt);
    // check if the actor is dead
    if (actor->getActorState() == HgeActor::state::DEAD) {
      deadActorMap_m.emplace(id, std::move(actor));
      activeActorMap_m.erase(id);
      // erase relevant model comp.
      if(actor->isRenderable())
        upHve_m->removeRenderableComponent(id);
    }
  }

  upCamera_m->update(dt);

  currentTime_m = newTime;
  isUpdating_m = false;

  // activate pending actor
  for (auto& pend : pendingActorMap_m) {
    activeActorMap_m.emplace(pend.first, std::move(pend.second));
  }
  pendingActorMap_m.clear();
  // clear all the dead actors
  deadActorMap_m.clear();

  upHve_m->render(dt, *(upCamera_m->viewerComponent()));
}

void HgeGame::render()
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
  auto path = std::string(std::getenv("HNLL_ENGN")) + modelDir;
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
  auto smoothVase = std::make_unique<HgeActor>();
  auto& smoothVaseHveModel = hveModelMap_m["smooth_vase"];
  auto smoothVaseModelComp = std::make_shared<ModelComponent>(smoothVase->getId(), smoothVaseHveModel);
  smoothVase->addRenderableComponent(smoothVaseModelComp);
  upHve_m->addRenderableComponent(smoothVaseModelComp);
  smoothVaseModelComp->setTranslation(glm::vec3{-0.5f, 0.5f, 0.f});
  smoothVaseModelComp->setScale(glm::vec3{3.f, 1.5f, 3.f});
  
  addActor(std::move(smoothVase));

  auto flatVase = std::make_unique<HgeActor>();
  auto& flatVaseHveModel = hveModelMap_m["flat_vase"];
  auto flatVaseModelComp = std::make_shared<ModelComponent>(flatVase->getId(), flatVaseHveModel);
  flatVase->addRenderableComponent(flatVaseModelComp);
  upHve_m->addRenderableComponent(flatVaseModelComp);
  flatVaseModelComp->setTranslation(glm::vec3{0.5f, 0.5f, 0.f});
  flatVaseModelComp->setScale(glm::vec3{3.f, 1.5f, 3.f});
  
  addActor(std::move(flatVase));

  auto floor = std::make_unique<HgeActor>();
  auto& floorHveModel = hveModelMap_m["quad"];
  auto floorModelComp = std::make_shared<ModelComponent>(floor->getId(), floorHveModel);
  floor->addRenderableComponent(floorModelComp);
  upHve_m->addRenderableComponent(floorModelComp);
  floorModelComp->setTranslation(glm::vec3{0.f, 0.5f, 0.f});
  floorModelComp->setScale(glm::vec3{3.f, 1.5f, 3.f});
  
  addActor(std::move(floor));

  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f},
      {.1f, .1f, 1.f},
      {.1f, 1.f, .1f},
      {1.f, 1.f, .1f},
      {.1f, 1.f, 1.f},
      {1.f, 1.f, 1.f} 
  };

  for (int i = 0; i < lightColors.size(); i++) {
    auto lightActor = std::make_unique<HgeActor>();
    auto lightComp = PointLightComponent::createPointLight(lightActor->getId(), 1, 0.0f, lightColors[i]);
    auto lightRotation = glm::rotate(
        glm::mat4(1),
        (i * glm::two_pi<float>()) / lightColors.size(),
        {0.f, -1.0f, 0.f}); // axiz
    lightComp->setTranslation(glm::vec3(lightRotation * glm::vec4(-1.f, -1.f, -1.f, 1.f)));
    lightActor->addRenderableComponent(lightComp);
    upHve_m->addRenderableComponent(lightComp);

    addActor(std::move(lightActor));    
  }
}

void HgeGame::cleanup()
{
  activeActorMap_m.clear();
  pendingActorMap_m.clear();
  deadActorMap_m.clear();
  hveModelMap_m.clear();
}

} // namespace hnll