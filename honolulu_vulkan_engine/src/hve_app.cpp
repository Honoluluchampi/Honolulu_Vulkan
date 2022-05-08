// this class organizes all the vulkan specific features

#include <hve_app.hpp>

#include <systems/point_light.hpp>
#include <systems/simple_renderer.hpp>

// lib
#include <glm/gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>
#include <iostream>

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
  auto simpleRendererSystem = std::make_unique<SimpleRendererSystem>(
    hveDevice_m, 
    hveRenderer_m.getSwapChainRenderPass(HVE_RENDER_PASS_ID),
    globalSetLayout_m->getDescriptorSetLayout());

  auto pointLightSystem = std::make_unique<PointLightSystem>(
    hveDevice_m, 
    hveRenderer_m.getSwapChainRenderPass(HVE_RENDER_PASS_ID),
    globalSetLayout_m->getDescriptorSetLayout());

  renderingSystems_m.emplace
    (simpleRendererSystem->getRenderType(), std::move(simpleRendererSystem));
  renderingSystems_m.emplace
    (pointLightSystem->getRenderType(), std::move(pointLightSystem));
}

// each render systems automatically detect render target components
void Hve::render(ViewerComponent& viewerComp)
{
  // returns nullptr if the swap chain is need to be recreated
  if (auto commandBuffer = hveRenderer_m.beginFrame()) {
    int frameIndex = hveRenderer_m.getFrameIndex();

    FrameInfo frameInfo{
        frameIndex, 
        commandBuffer, 
        globalDescriptorSets_m[frameIndex]
    };

    // update 
    ubo_.projection_m = viewerComp.getProjection();
    ubo_.view_m = viewerComp.getView();
    uboBuffers_m[frameIndex]->writeToBuffer(&ubo_);
    uboBuffers_m[frameIndex]->flush();

    // rendering
    // TODO : configure hve_render_pass_id as the 
    // member and detect it in beginSwapChainRenderPass func
    hveRenderer_m.beginSwapChainRenderPass(commandBuffer, HVE_RENDER_PASS_ID);
    // programmable stage of rendering
    // system can now access gameobjects via frameInfo
    for (auto& system : renderingSystems_m)
      system.second->render(frameInfo);

    hveRenderer_m.endSwapChainRenderPass(commandBuffer);
    hveRenderer_m.endFrame();
  }
}

void Hve::removeRenderableComponentWithoutOwner(RenderType type, HgeComponent::compId id)
{
  renderingSystems_m[type]->removeRenderTarget(id);
}
} // namespace hve