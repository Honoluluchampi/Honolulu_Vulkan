#include <hve_app.hpp>
#include <simple_renderer.hpp>

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

  while (!hveWindow_m.shouldClose()) {
    glfwPollEvents();

    // returns nullptr if the swap chain is need to be recreated
    if (auto commandBuffer = hveRenderer_m.beginFrame()) {
      // begin offscrean shadow pass
      // render shadow casting objects 
      // end offscreen shadow pass

      hveRenderer_m.beginSwapChainRenderPass(commandBuffer);

      // programmable stage of rendering
      simpleRendererSystem.renderGameObjects(commandBuffer, gameObjects_m);

      hveRenderer_m.endSwapChainRenderPass(commandBuffer);
      hveRenderer_m.endFrame();
    }
  }

  vkDeviceWaitIdle(hveDevice_m.device());
}

void HveApp::loadGameObjects()
{
  std::vector<HveModel::Vertex> vertices {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
  };

  auto hveModel = std::make_shared<HveModel>(hveDevice_m, vertices);
  
  auto triangle = HveGameObject::createGameObject();
  triangle.model_m = hveModel;
  triangle.color_m = {0.1f, 0.8f, 0.1f};
  triangle.transform2d_m.translation.x = 0.2f;
  triangle.transform2d_m.scale = {2.0f, 0.5f};
  triangle.transform2d_m.rotation = 0.25f * glm::two_pi<float>();

  gameObjects_m.push_back(std::move(triangle));
}
} // namespace hve