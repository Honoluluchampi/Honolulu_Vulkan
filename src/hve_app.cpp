#include <hve_app.hpp>

//std
#include <stdexcept>
#include <array>

namespace hve {

HveApp::HveApp()
{
  loadGameObjects();
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
  triangle.transform2d.translation.x = 0.2f;

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
  assert(hveSwapChain_m != nullptr && "cannot create pipeline before swap chain");
  assert(pipelineLayout_m != nullptr && "cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  HvePipeline::defaultPipelineConfigInfo(pipelineConfig);
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

  if (hveSwapChain_m == nullptr)
    hveSwapChain_m = std::make_unique<HveSwapChain>(hveDevice_m, extent);
  else {
    hveSwapChain_m = std::make_unique<HveSwapChain>(hveDevice_m, extent, std::move(hveSwapChain_m));
    if (hveSwapChain_m->imageCount() != commandBuffers_m.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }
  // if render pass compatible, do nothing else 
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

void HveApp::freeCommandBuffers()
{
  vkFreeCommandBuffers(
      hveDevice_m.device(), 
      hveDevice_m.getCommandPool(), 
      static_cast<float>(commandBuffers_m.size()), 
      commandBuffers_m.data());
  commandBuffers_m.clear();
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
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  // last parameter controls how the drawing commands within the render pass
  // will be provided. 
  vkCmdBeginRenderPass(commandBuffers_m[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  // dynamic viewport and scissor
  // transformation of the image                            
  // draw entire framebuffer
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(hveSwapChain_m->getSwapChainExtent().width);
  viewport.height = static_cast<float>(hveSwapChain_m->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  // cut the region of the framebuffer(swap chain)
  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = hveSwapChain_m->getSwapChainExtent();
  vkCmdSetViewport(commandBuffers_m[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(commandBuffers_m[imageIndex], 0, 1, &scissor);

  renderGameObjects(commandBuffers_m[imageIndex]);

  // finish render pass and recording the comand buffer
  vkCmdEndRenderPass(commandBuffers_m[imageIndex]);
  if (vkEndCommandBuffer(commandBuffers_m[imageIndex]) != VK_SUCCESS) 
    throw std::runtime_error("failed to record command buffer!");
}

void HveApp::renderGameObjects(VkCommandBuffer commandBuffer)
{
  hvePipeline_m->bind(commandBuffer);

  for (auto& obj : gameObjects_m) {
    SimplePushConstantData push{};
    push.offset_m = obj.transform2d.translation;
    push.color_m = obj.color_m;
    push.transform_m = obj.transform2d.mat2();

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