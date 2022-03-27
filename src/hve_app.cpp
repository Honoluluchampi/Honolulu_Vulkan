#include <hve_app.hpp>
#include <simple_renderer.hpp>
#include <hve_camera.hpp>
#include <keyboard_movement_controller.hpp>
#include <hve_buffer.hpp>

// lib
#include <glm/gtc/constants.hpp>

//std
#include <stdexcept>
#include <chrono>
#include <array>

namespace hve {

// global uniform buffer object
struct GlobalUbo
{
  glm::mat4 projectionView_m{1.f};
  glm::vec3 lightDirection_m = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
};

HveApp::HveApp()
{ 
  // 2 uniform buffer descriptor
  globalPool_m = HveDescriptorPool::Builder(hveDevice_m)
    .setMaxSets(HveSwapChain::MAX_FRAMES_IN_FLIGHT)
    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, HveSwapChain::MAX_FRAMES_IN_FLIGHT)
    .build();

  loadGameObjects(); 
}

HveApp::~HveApp()
{ }

void HveApp::run()
{
  // creating ubo for each frames version
  std::vector<std::unique_ptr<HveBuffer>> uboBuffers(HveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<HveBuffer>(
      hveDevice_m,
      sizeof(GlobalUbo),
      1,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );
    uboBuffers[i]->map();
  }

  // this is set layout of master system
  auto globalSetLayout = HveDescriptorSetLayout::Builder(hveDevice_m)
    .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
    .build();
  // may add additional layout of child system

  std::vector<VkDescriptorSet> globalDescriptorSets(HveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    HveDescriptorWriter(*globalSetLayout, *globalPool_m)
      .writeBuffer(0, &bufferInfo)
      .build(globalDescriptorSets[i]);
  }
  // creating one uniform buffer for all frames
  // HveBuffer globalUboBuffer {
  //   hveDevice_m,
  //   sizeof(GlobalUbo),
  //   HveSwapChain::MAX_FRAMES_IN_FLIGHT, // instance count
  //   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  //   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
  //   hveDevice_m.properties_m.limits.minUniformBufferOffsetAlignment,
  // };
  // globalUboBuffer.map();

  // create renderer system as local variable
  SimpleRendererSystem simpleRendererSystem {hveDevice_m, hveRenderer_m.getSwapChainRenderPass()};
  // create camera as ...
  HveCamera camera{};
  // camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
  // camera.setViewTarget(glm::vec3(-1.f, -2.f, 20.f), glm::vec3(0.f, 0.f, 2.5f));

  // object for change the camera transform indirectly
  // this object has no model and won't be rendered
  auto viewerObject = HveGameObject::createGameObject();
  KeyboardMovementController cameraController{};

  // for synchronization of the refresh rate
  auto currentTime = std::chrono::high_resolution_clock::now();

  while (!hveWindow_m.shouldClose()) {
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;
    frameTime = std::min(frameTime, MAX_FRAME_TIME);

    cameraController.moveInPlaneXZ(hveWindow_m.getGLFWwindow(), frameTime, viewerObject);
    camera.setViewYXZ(viewerObject.transform_m.translation_m, viewerObject.transform_m.rotation_m);

    float aspect = hveRenderer_m.getAspectRation();
    // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 50.f);
    // returns nullptr if the swap chain is need to be recreated
    if (auto commandBuffer = hveRenderer_m.beginFrame()) {
      int frameIndex = hveRenderer_m.getFrameIndex();

      FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera};

      // update 
      GlobalUbo ubo{};
      ubo.projectionView_m = camera.getProjection() * camera.getView();
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // rendering
      hveRenderer_m.beginSwapChainRenderPass(commandBuffer);
      // programmable stage of rendering
      simpleRendererSystem.renderGameObjects(frameInfo, gameObjects_m);
      hveRenderer_m.endSwapChainRenderPass(commandBuffer);
      hveRenderer_m.endFrame();
    }
  }

  vkDeviceWaitIdle(hveDevice_m.device());
}

void HveApp::loadGameObjects()
{
  std::shared_ptr<HveModel> hveModel = HveModel::createModelFromFile(hveDevice_m, "./models/flat_vase.obj");

  auto gameObj = HveGameObject::createGameObject();
  gameObj.model_m = hveModel;
  gameObj.transform_m.translation_m = {-0.5f, 0.5f, 2.5f};
  gameObj.transform_m.scale_m = {3.f, 1.5f, 3.f};
  gameObjects_m.push_back(std::move(gameObj));

  std::shared_ptr<HveModel> vaseModel = HveModel::createModelFromFile(hveDevice_m, "./models/smooth_vase.obj");

  auto vase = HveGameObject::createGameObject();
  vase.model_m = vaseModel;
  vase.transform_m.translation_m = {0.5f, 0.5f, 2.5f};
  vase.transform_m.scale_m = glm::vec3{3.f, 1.5f, 3.f};
  gameObjects_m.push_back(std::move(vase));
}
} // namespace hve