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
  for (int i = 0; i < uboBuffers_m.size(); i++) {
    uboBuffers_m[i] = std::make_unique<HveBuffer>(
      hveDevice_m,
      sizeof(GlobalUbo),
      1,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );
    uboBuffers_m[i]->map();
  }

  // this is set layout of master system
  // enable ubo to be referenced by oall stages of a graphics pipeline
  globalSetLayout_m = HveDescriptorSetLayout::Builder(hveDevice_m)
    .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
    .build();
  // may add additional layout of child system

  for (int i = 0; i < globalDescriptorSets_m.size(); i++) {
    auto bufferInfo = uboBuffers_m[i]->descriptorInfo();
    HveDescriptorWriter(*globalSetLayout_m, *globalPool_m)
      .writeBuffer(0, &bufferInfo)
      .build(globalDescriptorSets_m[i]);
  }

  // create renderer system as local variable
  simpleRendererSystem_m = std::make_unique<SimpleRendererSystem>(
    hveDevice_m, 
    hveRenderer_m.getSwapChainRenderPass(),
    globalSetLayout_m->getDescriptorSetLayout());

  pointLightSystem_m = std::make_unique<PointLightSystem>(
    hveDevice_m, 
    hveRenderer_m.getSwapChainRenderPass(),
    globalSetLayout_m->getDescriptorSetLayout());
  
  viewerObject_m.transform_m.translation_m.z = -2.5f;
}


void Hve::update(float dt)
{
  cameraController_m.moveInPlaneXZ(hveWindow_m.getGLFWwindow(), dt, viewerObject_m);
  camera_m.setViewYXZ(viewerObject_m.transform_m.translation_m, viewerObject_m.transform_m.rotation_m);
  float aspect = hveRenderer_m.getAspectRatio();
  camera_m.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 50.f);
}

void Hve::render(float dt, std::unordered_map<std::string, std::shared_ptr<ModelComponent>>& spModelCmpts)
{
  // returns nullptr if the swap chain is need to be recreated
  if (auto commandBuffer = hveRenderer_m.beginFrame()) {
    int frameIndex = hveRenderer_m.getFrameIndex();

    FrameInfo frameInfo{
        frameIndex, 
        dt, 
        commandBuffer, 
        camera_m, 
        globalDescriptorSets_m[frameIndex],
        spModelCmpts
    };

    // update 
    GlobalUbo ubo{};
    ubo.projection_m = camera_m.getProjection();
    ubo.view_m = camera_m.getView();
    pointLightSystem_m->update(frameInfo, ubo);
    uboBuffers_m[frameIndex]->writeToBuffer(&ubo);
    uboBuffers_m[frameIndex]->flush();

    // rendering
    hveRenderer_m.beginSwapChainRenderPass(commandBuffer);
    // programmable stage of rendering
    // system can now access gameobjects via frameInfo
    simpleRendererSystem_m->render(frameInfo);
    pointLightSystem_m->render(frameInfo);

    hveRenderer_m.endSwapChainRenderPass(commandBuffer);
    hveRenderer_m.endFrame();
  }
}
} // namespace hve