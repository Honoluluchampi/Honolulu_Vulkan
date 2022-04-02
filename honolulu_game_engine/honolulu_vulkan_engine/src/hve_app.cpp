// this class organizes all the vulkan specific features

#include <hve_app.hpp>


// lib
#include <glm/gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>

namespace hnll {

Hve::Hve(const char* windowName) : hveWindow_m{WIDTH, HEIGHT, windowName}
{
  init();
}

Hve::~Hve()
{ }

// todo : separate into some functions
void Hve::init()
{
  // // 2 uniform buffer descriptor
  globalPool_m = HveDescriptorPool::Builder(hveDevice_m)
    .setMaxSets(HveSwapChain::MAX_FRAMES_IN_FLIGHT)
    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, HveSwapChain::MAX_FRAMES_IN_FLIGHT)
    .build();

  // creating ubo for each frames version
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
  // enable ubo to be referenced by oall stages of a graphics pipeline
  globalSetLayout = HveDescriptorSetLayout::Builder(hveDevice_m)
    .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
    .build();
  // may add additional layout of child system

  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    HveDescriptorWriter(*globalSetLayout, *globalPool_m)
      .writeBuffer(0, &bufferInfo)
      .build(globalDescriptorSets[i]);
  }

  // create renderer system as local variable
  simpleRendererSystem = std::make_unique<SimpleRendererSystem>(
    hveDevice_m, 
    hveRenderer_m.getSwapChainRenderPass(),
    globalSetLayout->getDescriptorSetLayout());

  pointLightSystem = std::make_unique<PointLightSystem>(
    hveDevice_m, 
    hveRenderer_m.getSwapChainRenderPass(),
    globalSetLayout->getDescriptorSetLayout());
  
  viewerObject.transform_m.translation_m.z = -2.5f;
}

void Hve::render(float dt)
{
  cameraController.moveInPlaneXZ(hveWindow_m.getGLFWwindow(), dt, viewerObject);
  camera.setViewYXZ(viewerObject.transform_m.translation_m, viewerObject.transform_m.rotation_m);

  float aspect = hveRenderer_m.getAspectRation();
  // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
  camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 50.f);
  // returns nullptr if the swap chain is need to be recreated
  if (auto commandBuffer = hveRenderer_m.beginFrame()) {
    int frameIndex = hveRenderer_m.getFrameIndex();

    FrameInfo frameInfo{
        frameIndex, 
        dt, 
        commandBuffer, 
        camera, 
        globalDescriptorSets[frameIndex],
        gameObjects_m
    };

    // update 
    GlobalUbo ubo{};
    ubo.projection_m = camera.getProjection();
    ubo.view_m = camera.getView();
    pointLightSystem->update(frameInfo, ubo);
    uboBuffers[frameIndex]->writeToBuffer(&ubo);
    uboBuffers[frameIndex]->flush();

    // rendering
    hveRenderer_m.beginSwapChainRenderPass(commandBuffer);
    // programmable stage of rendering
    // system can now access gameobjects via frameInfo
    simpleRendererSystem->renderGameObjects(frameInfo);
    pointLightSystem->render(frameInfo);

    hveRenderer_m.endSwapChainRenderPass(commandBuffer);
    hveRenderer_m.endFrame();
  }
}

void Hve::createGameObjects(std::unordered_map<std::string, std::shared_ptr<HveModel>>& modelMap)
{
  std::shared_ptr<HveModel> smoothVaseModel = modelMap["smooth_vase"];
  auto gameObj = HveGameObject::createGameObject();
  gameObj.model_m = smoothVaseModel;
  gameObj.transform_m.translation_m = {-0.5f, 0.5f, 0.f};
  gameObj.transform_m.scale_m = {3.f, 1.5f, 3.f};
  // id is a key, HveGameObj is a value
  gameObjects_m.emplace(gameObj.getId(), std::move(gameObj));

  std::shared_ptr<HveModel> flatVaseModel = modelMap["flat_vase"];
  auto vase = HveGameObject::createGameObject();
  vase.model_m = flatVaseModel;
  vase.transform_m.translation_m = {0.5f, 0.5f, 0.f};
  vase.transform_m.scale_m = glm::vec3{3.f, 1.5f, 3.f};
  gameObjects_m.emplace(vase.getId(), std::move(vase));

  std::shared_ptr<HveModel> floorModel = modelMap["quad"];
  auto floor = HveGameObject::createGameObject();
  floor.model_m = floorModel;
  floor.transform_m.translation_m = {0.f, 0.5f, 0.f};
  floor.transform_m.scale_m = glm::vec3{3.f, 1.5f, 3.f};
  gameObjects_m.emplace(floor.getId(), std::move(floor));

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
    gameObjects_m.emplace(pointLight.getId(), std::move(pointLight));
  }

}
} // namespace hve