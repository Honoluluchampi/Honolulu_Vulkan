#include <hve_app.hpp>

// lib
#include <glm/gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>

namespace hve {

HveApp::HveApp()
{
  loadGameObjects();
  createPipelineLayout();
  createPipeline();
}

HveApp::~HveApp()
{
  vkDestroyPipelineLayout(hveDevice_m.device(), pipelineLayout_m, nullptr);
}

void HveApp::run()
{
  while (!hveWindow_m.shouldClose()) {
    glfwPollEvents();

    // returns nullptr if the swap chain is need to be recreated
    if (auto commandBuffer = hveRenderer_m.beginFrame()) {
      // begin offscrean shadow pass
      // render shadow casting objects 
      // end offscreen shadow pass
      
      hveRenderer_m.beginSwapChainRenderPass(commandBuffer);
      renderGameObjects(commandBuffer);
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

void HveApp::createPipelineLayout()
{
  // config push constant range
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  // mainly for if you are going to separate ranges for the vertex and fragment shaders
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(hveDevice_m.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout_m) != VK_SUCCESS)
      throw std::runtime_error("failed to create pipeline layout!");
}

void HveApp::createPipeline()
{
  assert(pipelineLayout_m != nullptr && "cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  HvePipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass_m = hveRenderer_m.getSwapChainRenderPass();
  pipelineConfig.pipelineLayout_m = pipelineLayout_m;
  hvePipeline_m = std::make_unique<HvePipeline>(
      hveDevice_m,
      "./shader/spv/vert.spv", 
      "./shader/spv/frag.spv",
      pipelineConfig);
}


void HveApp::renderGameObjects(VkCommandBuffer commandBuffer)
{
  hvePipeline_m->bind(commandBuffer);

  for (auto& obj : gameObjects_m) {
    obj.transform2d_m.rotation = glm::mod(obj.transform2d_m.rotation + 0.01f, glm::two_pi<float>());

    SimplePushConstantData push{};
    push.offset_m = obj.transform2d_m.translation;
    push.color_m = obj.color_m;
    push.transform_m = obj.transform2d_m.mat2();

    vkCmdPushConstants(
        commandBuffer,
        pipelineLayout_m, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        0, 
        sizeof(SimplePushConstantData), 
        &push);
    obj.model_m->bind(commandBuffer);
    obj.model_m->draw(commandBuffer);
  }
}

} // namespace hve