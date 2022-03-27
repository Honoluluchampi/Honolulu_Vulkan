#include <hve_renderer.hpp>

//std
#include <stdexcept>
#include <array>

namespace hve {

HveRenderer::HveRenderer(HveWindow& window, HveDevice& device) : hveWindow_m{window}, hveDevice_m{device}
{
  recreateSwapChain();
  createCommandBuffers();
}

HveRenderer::~HveRenderer()
{
  freeCommandBuffers();
}

void HveRenderer::recreateSwapChain()
{
  // stop calculation until the window is minimized
  auto extent = hveWindow_m.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = hveWindow_m.getExtent();
    glfwWaitEvents();
  }
  // wait for finishing the current task
  vkDeviceWaitIdle(hveDevice_m.device());

  // for first creation
  if (hveSwapChain_m == nullptr)
    hveSwapChain_m = std::make_unique<HveSwapChain>(hveDevice_m, extent);
  // recreate
  else {
    // move the ownership of the current swap chain to old one.
    std::shared_ptr<HveSwapChain> oldSwapChain = std::move(hveSwapChain_m);
    hveSwapChain_m = std::make_unique<HveSwapChain>(hveDevice_m, extent, oldSwapChain);

    if (!oldSwapChain->compareSwapChainFormats(*hveSwapChain_m.get()))
      throw std::runtime_error("swap chian image( or depth) format has chainged");

    // command buffers no longer depend on the swap chain image count
  }
  // if render pass compatible, do nothing else 
}

void HveRenderer::createCommandBuffers() 
{
  // 2 or 3
  commandBuffers_m.resize(HveSwapChain::MAX_FRAMES_IN_FLIGHT);

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

void HveRenderer::freeCommandBuffers()
{
  vkFreeCommandBuffers(
      hveDevice_m.device(), 
      hveDevice_m.getCommandPool(), 
      static_cast<float>(commandBuffers_m.size()), 
      commandBuffers_m.data());
  commandBuffers_m.clear();
}

VkCommandBuffer HveRenderer::beginFrame()
{
  assert(!isFrameStarted_m && "Can't call beginFrame() while already in progress");
  // get finished image from swap chain
  auto result = hveSwapChain_m->acquireNextImage(&currentImageIndex_m);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    // the frame has not successfully started
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    throw std::runtime_error("failed to acquire swap chain image!");

  isFrameStarted_m = true;

  auto commandBuffer = getCurrentCommandBuffer();
  assert(commandBuffer != VK_NULL_HANDLE && "");
    // start reconding command buffers
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // // how to use the command buffer
  // beginInfo.flags = 0;
  // // state to inherit from the calling primary command buffers
  // // (only relevant for secondary command buffers)
  // beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
    throw std::runtime_error("failed to begin recording command buffer!");
  return commandBuffer;
}

void HveRenderer::endFrame()
{
  assert(isFrameStarted_m && "Can't call endFrame() while the frame is not in progress");
  auto commandBuffer = getCurrentCommandBuffer();
  // end recording command buffer
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
    throw std::runtime_error("failed to record command buffer!");

  auto result = hveSwapChain_m->submitCommandBuffers(&commandBuffer, &currentImageIndex_m);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || hveWindow_m.wasWindowResized()) {
    hveWindow_m.resetWindowResizedFlag();
    recreateSwapChain();
  }
  else if (result != VK_SUCCESS)
    throw std::runtime_error("failed to present swap chain image!");

  isFrameStarted_m = false;
  // increment currentFrameIndex_m
  if (++currentFrameIndex_m == HveSwapChain::MAX_FRAMES_IN_FLIGHT)
    currentFrameIndex_m = 0;
}

void HveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
  assert(isFrameStarted_m && "Can't call beginSwapChainRenderPass() while the frame is not in progress.");
  assert(commandBuffer == getCurrentCommandBuffer() && "Can't beginig render pass on command buffer from a different frame.");

  // starting a render pass
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = hveSwapChain_m->getRenderPass();
  renderPassInfo.framebuffer = hveSwapChain_m->getFrameBuffer(currentImageIndex_m);
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
  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void HveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
  assert(isFrameStarted_m && "Can't call endSwapChainRenderPass() while the frame is not in progress.");
  assert(commandBuffer == getCurrentCommandBuffer() && "Can't ending render pass on command buffer from a different frame.");

  // finish render pass and recording the comand buffer
  vkCmdEndRenderPass(commandBuffer);
}

} // namespace hve