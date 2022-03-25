#include <hve_app.hpp>

//std
#include <stdexcept>
#include <array>

namespace hve {

HveApp::HveApp()
{
  loadModels();
  createPipelineLayout();
  recreateSwapChain();
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
    drawFrame();
  }

  vkDeviceWaitIdle(hveDevice_m.device());
}

void HveApp::loadModels()
{
  std::vector<HveModel::Vertex> vertices {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
  };

  hveModel_m = std::make_unique<HveModel>(hveDevice_m, vertices);
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
  PipelineConfigInfo pipelineConfig{};
  HvePipeline::defaultPipelineConfigInfo(pipelineConfig, hveSwapChain_m->width(), hveSwapChain_m->height());
  pipelineConfig.renderPass_m = hveSwapChain_m->getRenderPass();
  pipelineConfig.pipelineLayout_m = pipelineLayout_m;
  hvePipeline_m = std::make_unique<HvePipeline>(
      hveDevice_m,
      "./shader/spv/vert.spv", 
      "./shader/spv/frag.spv",
      pipelineConfig);
}

void HveApp::recreateSwapChain()
{
  // stop calculation until the window is minimized
  auto extent = hveWindow_m.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = hveWindow_m.getExtent();
    glfwWaitEvents();
  }
  // wait for finishing the current task
  vkDeviceWaitIdle(hveDevice_m.device());
  hveSwapChain_m = std::make_unique<HveSwapChain>(hveDevice_m, extent);
  createPipeline();
}

void HveApp::createCommandBuffers() 
{
  // 2 or 3
  commandBuffers_m.resize(hveSwapChain_m->imageCount());

  // specify command pool and number of buffers to allocate
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  // if the allocated command buffers are primary or secondary command buffers
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = hveDevice_m.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_m.size());
  
  if (vkAllocateCommandBuffers(hveDevice_m.device(), &allocInfo, commandBuffers_m.data()) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate command buffers!");

}

void HveApp::recordCommandBuffer(int imageIndex)
{
  // start reconding command buffers
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // // how to use the command buffer
  // beginInfo.flags = 0;
  // // state to inherit from the calling primary command buffers
  // // (only relevant for secondary command buffers)
  // beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(commandBuffers_m[imageIndex], &beginInfo) != VK_SUCCESS) 
    throw std::runtime_error("failed to begin recording command buffer!");

      // starting a render pass
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = hveSwapChain_m->getRenderPass();
  renderPassInfo.framebuffer = hveSwapChain_m->getFrameBuffer(imageIndex);
  // the pixels outside this region will have undefined values
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = hveSwapChain_m->getSwapChainExtent();

  // default value for color and depth
  std::array<VkClearValue, 2> clearValues;
  clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  // last parameter controls how the drawing commands within the render pass
  // will be provided. 
  vkCmdBeginRenderPass(commandBuffers_m[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  hvePipeline_m->bind(commandBuffers_m[imageIndex]);

  // // bind the vertex buffer
  // VkBuffer vertexBuffers[] = {vertexBuffer};
  // VkDeviceSize offsets[] = {0};
  // vkCmdBindVertexBuffers(commandBuffers_m[i], 0, 1, vertexBuffers, offsets);

  // draw triangle
  // vertexCount, instanceCount, firstVertex, firstInstance
  // vkCmdDraw(commandBuffers_m[i], 3, 1, 0, 0);

  hveModel_m->bind(commandBuffers_m[imageIndex]);
  hveModel_m->draw(commandBuffers_m[imageIndex]);

  // finish render pass and recording the comand buffer
  vkCmdEndRenderPass(commandBuffers_m[imageIndex]);
  if (vkEndCommandBuffer(commandBuffers_m[imageIndex]) != VK_SUCCESS) 
    throw std::runtime_error("failed to record command buffer!");
}

void HveApp::drawFrame() 
{
  uint32_t imageIndex;
  // get finished image from swap chain
  auto result = hveSwapChain_m->acquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    throw std::runtime_error("failed to acquire swap chain image!");

  recordCommandBuffer(imageIndex);

  result = hveSwapChain_m->submitCommandBuffers(&commandBuffers_m[imageIndex], &imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || hveWindow_m.wasWindowResized()) {
    hveWindow_m.resetWindowResizedFlag();
    recreateSwapChain();
    return;
  }
  if (result != VK_SUCCESS)
    throw std::runtime_error("failed to present swap chain image!");
}

} // namespace hve