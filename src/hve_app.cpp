#include <hve_app.hpp>
#include <simple_renderer.hpp>
#include <hve_camera.hpp>

// lib
#include <glm/gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>

namespace hve {

HveApp::HveApp()
{
  loadGameObjects();
}

HveApp::~HveApp()
{
}

void HveApp::run()
{
  // create renderer system as local variable
  SimpleRendererSystem simpleRendererSystem {hveDevice_m, hveRenderer_m.getSwapChainRenderPass()};
  // create camera as ...
  HveCamera camera{};

  while (!hveWindow_m.shouldClose()) {
    glfwPollEvents();
    float aspect = hveRenderer_m.getAspectRation();
    // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
    // returns nullptr if the swap chain is need to be recreated
    if (auto commandBuffer = hveRenderer_m.beginFrame()) {
      // begin offscrean shadow pass
      // render shadow casting objects 
      // end offscreen shadow pass

      hveRenderer_m.beginSwapChainRenderPass(commandBuffer);

      // programmable stage of rendering
      simpleRendererSystem.renderGameObjects(commandBuffer, gameObjects_m, camera);

      hveRenderer_m.endSwapChainRenderPass(commandBuffer);
      hveRenderer_m.endFrame();
    }
  }

  vkDeviceWaitIdle(hveDevice_m.device());
}

// temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<HveModel> createCubeModel(HveDevice& device, glm::vec3 offset) {
  std::vector<HveModel::Vertex> vertices{
 
      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
 
      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
 
      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
 
      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
 
      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
 
      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
 
  };
  for (auto& v : vertices) {
    v.position_m += offset;
  }
  return std::make_unique<HveModel>(device, vertices);
}

void HveApp::loadGameObjects()
{
  std::shared_ptr<HveModel> hveModel = createCubeModel(hveDevice_m, {.0f, .0f, .0f});

  auto cube = HveGameObject::createGameObject();
  cube.model_m = hveModel;
  cube.transform_m.translation_m = {0.0f, 0.0f, 2.5f};
  cube.transform_m.scale_m = {.5f, .5f, .5f};
  gameObjects_m.push_back(std::move(cube));
}
} // namespace hve