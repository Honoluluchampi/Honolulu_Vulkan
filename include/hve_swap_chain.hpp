#pragma once

#include "hve_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>

namespace hve {

class HveSwapChain {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  HveSwapChain(HveDevice &deviceRef, VkExtent2D windowExtent);
  ~HveSwapChain();

  HveSwapChain(const HveSwapChain &) = delete;
  HveSwapChain& operator=(const HveSwapChain &) = delete;

  VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers_m[index]; }
  VkRenderPass getRenderPass() { return renderPass_m; }
  VkImageView getImageView(int index) { return swapChainImageViews_m[index]; }
  size_t imageCount() { return swapChainImages_m.size(); }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat_m; }
  VkExtent2D getSwapChainExtent() { return swapChainExtent_m; }
  uint32_t width() { return swapChainExtent_m.width; }
  uint32_t height() { return swapChainExtent_m.height; }

  float extentAspectRatio() 
  { return static_cast<float>(swapChainExtent_m.width) / static_cast<float>(swapChainExtent_m.height); }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

 private:
  void createSwapChain();
  void createImageViews();
  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();

  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats);
  // most important settings for swap chain
  VkPresentModeKHR chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes);
  // choose resolution of output
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat swapChainImageFormat_m;
  VkExtent2D swapChainExtent_m;

  std::vector<VkFramebuffer> swapChainFramebuffers_m;
  // tell Vulkan about the framebuffer attachments that will be used while rendering
  // how many color and depth buffers there will be
  // how many samples to use for each of them
  VkRenderPass renderPass_m;

  std::vector<VkImage> depthImages_m;
  std::vector<VkDeviceMemory> depthImageMemorys_m;
  std::vector<VkImageView> depthImageViews_m;
  std::vector<VkImage> swapChainImages_m;
  std::vector<VkImageView> swapChainImageViews_m;

  HveDevice &device_m;
  VkExtent2D windowExtent_m;

  VkSwapchainKHR swapChain_m;

  // an image has been acquired and is ready for rendering
  std::vector<VkSemaphore> imageAvailableSemaphores_m;
  // rendering has finished and presentation can happen
  std::vector<VkSemaphore> renderFinishedSemaphores_m;
  // to use the right pair of semaphores every time
  size_t currentFrame_m = 0;
  // for CPU-GPU synchronization
  std::vector<VkFence> inFlightFences_m;
  // wait on before a new frame can use that image
  std::vector<VkFence> imagesInFlight_m;
};

}  // namespace lve