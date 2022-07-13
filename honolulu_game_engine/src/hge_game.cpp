#include <hge_game.hpp>

// std
#include <filesystem>
#include <iostream>
#include <typeinfo>

namespace hnll {

constexpr float MAX_FPS = 30.0f;
constexpr float MAX_DT = 0.05f;

// static members
HgeActor::map HgeGame::pendingActorMap_m;

// glfw
GLFWwindow* HgeGame::glfwWindow_m;
std::vector<u_ptr<std::function<void(GLFWwindow*, int, int, int)>>> HgeGame::glfwMouseButtonCallbacks_{};

// x12
Display* HgeGame::display_ = XOpenDisplay(NULL);

HgeGame::HgeGame(const char* windowName) : upHve_m(std::make_unique<Hve>(windowName))
{
  setGLFWwindow(); // ?

#ifndef __IMGUI_DISABLED
  upHie_m = std::make_unique<Hie>
    (upHve_m->hveWindow(), upHve_m->hveDevice());
  // configure dependency between renderers
  upHve_m->hveRenderer().setNextRenderer(upHie_m->pHieRenderer());  
#endif

  initHgeActors();
  loadData();

  // glfw
  setGlfwMouseButtonCallbacks();
}

HgeGame::~HgeGame()
{
  // cleanup in HgeGame::cleanup();
  // HveRenderer::cleanupSwapChain();
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

void HgeGame::update()
{
  isUpdating_m = true;

  float dt;
  std::chrono::_V2::system_clock::time_point newTime;
  // calc dt
  do {
  newTime = std::chrono::high_resolution_clock::now();

  dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime_m).count();
  } while(dt < 1.0f / MAX_FPS);

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
      // TODO dont use hgeActor::id_t but HgeComponent::id_t
      // if(actor->isRenderable())
        // upHve_m->removeRenderableComponent(id);
    }
  }

  // game specific update
  updateGame(dt);

  // camera
  upCamera_m->update(dt);
  upLightManager_->update(dt);

  currentTime_m = newTime;
  isUpdating_m = false;

  // activate pending actor
  for (auto& pend : pendingActorMap_m) {
    if(pend.second->isRenderable())
      upHve_m->addRenderableComponent(pend.second->getRenderableComponent());
    activeActorMap_m.emplace(pend.first, std::move(pend.second));
  }
  pendingActorMap_m.clear();
  // clear all the dead actors
  deadActorMap_m.clear();

  // TODO : delete gui demo
  // if (renderableComponentID_m != -1)
  //   HgeRenderableComponent& comp = dynamic_cast<HgeRenderableComponent&>(activeActorMap_m[hieModelID_]->getRenderableComponent());
  // upHie_m->update(comp.getTransform().translation_m);
}

void HgeGame::render()
{

  upHve_m->render(*(upCamera_m->viewerComponent()));
#ifndef __IMGUI_DISABLED
  if (!HveRenderer::swapChainRecreated_m){
    upHie_m->beginImGui();
    updateImgui();
    upHie_m->render();
  }
#endif
}

#ifndef __IMGUI_DISABLED
void HgeGame::updateImgui()
{
  // some general imgui upgrade
  updateGameImgui();
  for (auto& kv : activeActorMap_m) {
  const id_t& id = kv.first;
  auto& actor = kv.second;
  actor->updateImgui();
  }
}
#endif

void HgeGame::initHgeActors()
{
  // hge actors
  upCamera_m = std::make_shared<HgeCamera>(*upHve_m);
  
  // TODO : configure priorities of actors, then update light manager after all light comp
  upLightManager_ = std::make_shared<HgePointLightManager>(upHve_m->globalUbo());

}

void HgeGame::loadData()
{
  // load raw data
  loadHveModels();
  // temporary
  // loadActor();
}

// use filenames as the key of the map
// TODO : add models by adding folders or files
void HgeGame::loadHveModels(const std::string& modelDir)
{
  // auto path = std::string(std::getenv("HNLL_ENGN")) + modelDir;
  auto path = std::string("/home/honolulu/models/primitives");
  for (const auto & file : std::filesystem::directory_iterator(path)) {
    auto filename = std::string(file.path());
    auto length = filename.size() - path.size() - 5;
    auto key = filename.substr(path.size() + 1, length);
    auto hveModel = HveModel::createModelFromFile(upHve_m->hveDevice(), filename);
    hveModelMap_m.emplace(key, std::move(hveModel));
  }
}

// actors should be created as shared_ptr
void HgeGame::addActor(const s_ptr<HgeActor>& actor)
{ pendingActorMap_m.emplace(actor->getId(), actor); }

// void HgeGame::addActor(s_ptr<HgeActor>&& actor)
// { pendingActorMap_m.emplace(actor->getId(), std::move(actor)); }

void HgeGame::removeActor(id_t id)
{
  pendingActorMap_m.erase(id);
  activeActorMap_m.erase(id);
  // renderableActorMap_m.erase(id);
}

void HgeGame::loadActor()
{
  auto smoothVase = createActor();
  auto& smoothVaseHveModel = hveModelMap_m["smooth_vase"];
  auto smoothVaseModelComp = std::make_shared<MeshComponent>(smoothVase->getId(), smoothVaseHveModel);
  smoothVase->addRenderableComponent(smoothVaseModelComp);
  smoothVaseModelComp->setTranslation(glm::vec3{-0.5f, 0.5f, 0.f});
  smoothVaseModelComp->setScale(glm::vec3{3.f, 1.5f, 3.f});
  
  // temporary
  hieModelID_ = smoothVase->getId();

  auto flatVase = createActor();
  auto& flatVaseHveModel = hveModelMap_m["flat_vase"];
  auto flatVaseModelComp = std::make_shared<MeshComponent>(flatVase->getId(), flatVaseHveModel);
  flatVase->addRenderableComponent(flatVaseModelComp);
  flatVaseModelComp->setTranslation(glm::vec3{0.5f, 0.5f, 0.f});
  flatVaseModelComp->setScale(glm::vec3{3.f, 1.5f, 3.f});
  
  auto floor = createActor();
  auto& floorHveModel = hveModelMap_m["quad"];
  auto floorModelComp = std::make_shared<MeshComponent>(floor->getId(), floorHveModel);
  floor->addRenderableComponent(floorModelComp);
  floorModelComp->setTranslation(glm::vec3{0.f, 0.5f, 0.f});
  floorModelComp->setScale(glm::vec3{3.f, 1.5f, 3.f});

  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f},
      {.1f, .1f, 1.f},
      {.1f, 1.f, .1f},
      {1.f, 1.f, .1f},
      {.1f, 1.f, 1.f},
      {1.f, 1.f, 1.f} 
  };

  for (int i = 0; i < lightColors.size(); i++) {
    auto lightActor = createActor();
    auto lightComp = PointLightComponent::createPointLight(lightActor->getId(), 1.0f, 0.f, lightColors[i]);
    auto lightRotation = glm::rotate(
        glm::mat4(1),
        (i * glm::two_pi<float>()) / lightColors.size(),
        {0.f, -1.0f, 0.f}); // axiz
    lightComp->setTranslation(glm::vec3(lightRotation * glm::vec4(-1.f, -1.f, -1.f, 1.f)));
    addPointLight(lightActor, lightComp);
  }
}

void HgeGame::addPointLight(s_ptr<HgeActor>& owner, s_ptr<PointLightComponent>& lightComp)
{
  // shared by three actor 
  owner->addRenderableComponent(lightComp);
  upLightManager_->addLightComp(lightComp);
} 

void HgeGame::addPointLightWithoutOwner(s_ptr<PointLightComponent>& lightComp)
{
  // path to the renderer
  upHve_m->addRenderableComponent(lightComp);
  // path to the manager
  upLightManager_->addLightComp(lightComp);
}

void HgeGame::removePointLightWithoutOwner(HgeComponent::compId id)
{
  upHve_m->removeRenderableComponentWithoutOwner(RenderType::POINT_LIGHT, id);
  upLightManager_->removeLightComp(id);
}


void HgeGame::cleanup()
{
  activeActorMap_m.clear();
  pendingActorMap_m.clear();
  deadActorMap_m.clear();
  hveModelMap_m.clear();
  HveRenderer::cleanupSwapChain();
}

// glfw
void HgeGame::setGlfwMouseButtonCallbacks()
{
  glfwSetMouseButtonCallback(glfwWindow_m, glfwMouseButtonCallback);
}

void HgeGame::addGlfwMouseButtonCallback(u_ptr<std::function<void(GLFWwindow* window, int button, int action, int mods)>>&& func)
{
  glfwMouseButtonCallbacks_.emplace_back(std::move(func));
  setGlfwMouseButtonCallbacks();
}

void HgeGame::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  for (const auto& func : glfwMouseButtonCallbacks_)
    func->operator()(window, button, action, mods);
  
#ifndef __IMGUI_DISABLED
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
#endif
}

} // namespace hnll