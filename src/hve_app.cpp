#include <hve_app.hpp>

//std
#include <stdexcept>

namespace hve {

HveApp::HveApp()
{
  createPipelineLayout();
  createPipeline();
  createCommandBuffers();
}

HveApp::~HveApp()
{
  vkDestroyPipelineLayout(hveDevice_m.device(), pipelineLayout_m, nullptr);
}

void HveApp::run()
{
  while (!hveWindow_m.shouldClose()) {
    glfwPollEvents();
  }
}

void HveApp::createPipelineLayout()
{
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;
  if (vkCreatePipelineLayout(hveDevice_m.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout_m) != VK_SUCCESS)
      throw std::runtime_error("failed to create pipeline layout!");
}

void HveApp::createPipeline()
{
  auto pipelineConfig = HvePipeline::defaultPipelineConfigInfo(hveSwapChain.width(), hveSwapChain.height());
  pipelineConfig.renderPass_m = hveSwapChain.getRenderPass();
  pipelineConfig.pipelineLayout_m = pipelineLayout_m;
  hvePipeline_m = std::make_unique<HvePipeline>(
      hveDevice_m,
      "./shader/spv/vert.spv", 
      "./shader/spv/frag.spv",
      pipelineConfig);
}

void HveApp::createCommandBuffers() {}
void HveApp::drawFrame() {}

} // namespace hve