// this class organizes all the graphics specific features
// hnll
#include <graphics/engine.hpp>
#include <graphics/systems/point_light.hpp>
#include <graphics/systems/mesh_rendering_system.hpp>
#include <graphics/systems/line_rendering_system.hpp>

//std
#include <stdexcept>
#include <array>
#include <iostream>

// lib
#include <glm/gtc/constants.hpp>

namespace hnll {

engine::engine(const char* windowName) : hveWindow_m{WIDTH, HEIGHT, windowName}
{
  init();
}

engine::~engine()
{ }

// todo : separate into some functions
void engine::init()
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
      sizeof(global_ubo),
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
  auto meshRenderingSystem = std::make_unique<MeshRenderingSystem>(
    hveDevice_m, 
    renderer_.getSwapChainRenderPass(HVE_RENDER_PASS_ID),
    globalSetLayout_m->getDescriptorSetLayout()
  );

  auto pointLightSystem = std::make_unique<PointLightSystem>(
    hveDevice_m, 
    renderer_.getSwapChainRenderPass(HVE_RENDER_PASS_ID),
    globalSetLayout_m->getDescriptorSetLayout()
  );

  auto lineRenderingSystem = std::make_unique<LineRenderingSystem>(
    hveDevice_m,
    renderer_.getSwapChainRenderPass(HVE_RENDER_PASS_ID),
    globalSetLayout_m->getDescriptorSetLayout()
  );

  renderingSystems_m.emplace
    (meshRenderingSystem->get_render_type(), std::move(meshRenderingSystem));
  renderingSystems_m.emplace
    (pointLightSystem->get_render_type(), std::move(pointLightSystem));
  renderingSystems_m.emplace
    (lineRenderingSystem->get_render_type(), std::move(lineRenderingSystem));
}

// each render systems automatically detect render target components
void engine::render(viewer_component& viewerComp)
{
  // returns nullptr if the swap chain is need to be recreated
  if (auto commandBuffer = renderer_.beginFrame()) {
    int frameIndex = renderer_.getFrameIndex();

    FrameInfo frameInfo{
        frameIndex, 
        commandBuffer, 
        globalDescriptorSets_m[frameIndex]
    };

    // update 
    ubo_.projection_m = viewerComp.get_projection();
    ubo_.view_m = viewerComp.get_view();
    uboBuffers_m[frameIndex]->writeToBuffer(&ubo_);
    uboBuffers_m[frameIndex]->flush();

    // rendering
    // TODO : configure hve_render_pass_id as the 
    // member and detect it in beginSwapChainRenderPass func
    renderer_.beginSwapChainRenderPass(commandBuffer, HVE_RENDER_PASS_ID);
    // programmable stage of rendering
    // system can now access gameobjects via frameInfo
    for (auto& system : renderingSystems_m)
      system.second->render(frameInfo);

    renderer_.endSwapChainRenderPass(commandBuffer);
    renderer_.endFrame();
  }
}

void engine::remove_renderable_component_without_owner(render_type type, component::id id)
{
  renderingSystems_m[type]->removeRenderTarget(id);
}
} // namespace hve