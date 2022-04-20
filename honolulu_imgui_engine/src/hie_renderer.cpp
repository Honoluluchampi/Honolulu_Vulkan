#include <hie_renderer.hpp>

namespace hnll {

HieRenderer::HieRenderer(HveDevice& hveDevice, HveSwapChain& hveSwapChain) : 
  device_(hveDevice.device()), graphicsFamilyIndex_(hveDevice.queueFamilyIndices().graphicsFamily_m.value()), hveSwapChain_(hveSwapChain)
{
  createCommandPool();
  createCommandBuffers();
}

HieRenderer::~HieRenderer()
{
  freeCommandBuffers();
  freeCommandPool();
}

VkCommandBuffer HieRenderer::beginFrame()
{
  assert(!isFrameStarted_m && "can't call beginFrame() while already in progress");
  auto result = hveSwapChain_.acquireNextImage(&currentImageIndex_);

  // only hveRenderer can recreate swap chain
  if (result == VK_ERROR_OUT_OF_DATE_KHR)
    return nullptr;

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    throw std::runtime_error("failed to acquire swap chain image!");

  isFrameStarted_m = true;

  auto commandBuffer = getCurrentCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    throw std::runtime_error("failed to begin recording command buffer!");
  
  return commandBuffer;
}

void HieRenderer::endFrame()
{
  assert(isFrameStarted_m && "can't call endFrame() while the frame is not in progress.");
  auto commandBuffer = getCurrentCommandBuffer();
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    throw std::runtime_error("failed to record command buffer!");
  
  // only hverenderer can recreate swap chain
  auto result = hveSwapChain_.submitCommandBuffers(&commandBuffer, &currentImageIndex_);
  if (result != VK_SUCCESS) 
    throw std::runtime_error("failed to present swap chain image!");

  isFrameStarted_m = false;
  if (++currentFrameIndex_ == HveSwapChain::MAX_FRAMES_IN_FLIGHT)
    currentFrameIndex_ = 0;
}

void HieRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
  
}

void HieRenderer::createCommandBuffers()
{
  // 2 or 3
  commandBuffers_.resize(HveSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  info.commandPool = commandPool_;
  info.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

  if (vkAllocateCommandBuffers(device_, &info, commandBuffers_.data()) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate command buffers!");
}

void HieRenderer::createCommandPool()
{
  VkCommandPoolCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.queueFamilyIndex = graphicsFamilyIndex_;
  info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(device_, &info, nullptr, &commandPool_) != VK_SUCCESS)
    throw std::runtime_error("could not create grahpics command pool.");
}

void HieRenderer::freeCommandBuffers()
{
  vkFreeCommandBuffers(device_, commandPool_, static_cast<float>(commandBuffers_.size()), commandBuffers_.data());
  commandBuffers_.clear();
}

void HieRenderer::freeCommandPool()
{
  vkDestroyCommandPool(device_, commandPool_, nullptr);
}

} // namespace hnll