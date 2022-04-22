#pragma once

#include "hve_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace hnll {

class HveSwapChain {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  HveSwapChain(HveDevice &deviceRef, VkExtent2D windowExtent);
  HveSwapChain(HveDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<HveSwapChain> previous);
  ~HveSwapChain();

  HveSwapChain(const HveSwapChain &) = delete;
  HveSwapChain& operator=(const HveSwapChain &) = delete;

#ifdef IMGUI_DISABLED
  VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers_m[index]; }
  VkRenderPass getRenderPass() { return renderPass_m; }
#else
  VkFramebuffer getFramebuffer(int renderPassId, int index)
  { return multipleFramebuffers_m[renderPassId][index]; }
  VkRenderPass getRenderPass(int renderPassId)
  { return multipleRenderPass_m[renderPassId]; }
#endif

  VkImageView getImageView(int index) { return swapChainImageViews_m[index]; }
  size_t imageCount() { return swapChainImages_m.size(); }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat_m; }
  VkExtent2D getSwapChainExtent() { return swapChainExtent_m; }
  uint32_t width() { return swapChainExtent_m.width; }
  uint32_t height() { return swapChainExtent_m.height; }
  const VkSemaphore& getCurrentImageAvailableSemaphore() const
  { return imageAvailableSemaphores_m[currentFrame_m]; }
  const VkSemaphore& getCurrentRenderFinishedSemaphore() const
  { return renderFinishedSemaphores_m[currentFrame_m]; }
  const VkFence& getCurrentInFlightFence() const 
  { return inFlightFences_m[currentFrame_m]; }
  const VkFence& getCurrentImagesInFlightFence() const
  { return imagesInFlight_m[currentFrame_m]; }

  float extentAspectRatio() 
  { return static_cast<float>(swapChainExtent_m.width) / static_cast<float>(swapChainExtent_m.height); }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

  // swap chain validation whether its compatible with the renderPass
  bool compareSwapChainFormats(const HveSwapChain& swapChain) const 
  { return (swapChain.swapChainDepthFormat_m == swapChainDepthFormat_m) && (swapChain.swapChainImageFormat_m == swapChainImageFormat_m); }

  void setRenderPass(VkRenderPass renderPass, int renderPassId)
  { multipleRenderPass_m[renderPassId] = renderPass; }

  // VFB : std::vector<VkFramebuffer>
  template <class VFB>
  void setFramebuffers(VFB&& framebuffers, int renderPassId)
  { multipleFramebuffers_m[renderPassId] = std::forward<VFB>(framebuffers); }

 private:
  void init();
  void createSwapChain();
  void createImageViews();
  void createDepthResources();
  void createSyncObjects();

  VkRenderPass createRenderPass();
  std::vector<VkFramebuffer> createFramebuffers(VkRenderPass renderPass);

#ifndef __IMGUI_DISABLED
  void createMultipleFramebuffers();
  void createMultipleRenderPass();
  // to add imgui renderPass
  void addRenderPass(const VkRenderPass& renderPass)
  { multipleRenderPass_m.emplace_back(renderPass); }
  // to add imgui frame buffers
  void addFramebuffers(const std::vector<VkFramebuffer>& framebuffers)
  { multipleFramebuffers_m.emplace_back(framebuffers); }
#endif

  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats);
  // most important settings for swap chain
  VkPresentModeKHR chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes);
  // choose resolution of output
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat swapChainImageFormat_m;
  VkFormat swapChainDepthFormat_m;
  VkExtent2D swapChainExtent_m;

  // tell Vulkan about the framebuffer attachments that will be used while rendering
  // how many color and depth buffers there will be
  // how many samples to use for each of them
#ifdef __IMGUI_DISABLED
  std::vector<VkFramebuffer> swapChainFramebuffers_m;
  VkRenderPass renderPass_m;
#else
  std::vector<std::vector<VkFramebuffer>> multipleFramebuffers_m;
  std::vector<VkRenderPass> multipleRenderPass_m;
#endif

  std::vector<VkImage> depthImages_m;
  std::vector<VkDeviceMemory> depthImageMemorys_m;
  std::vector<VkImageView> depthImageViews_m;
  std::vector<VkImage> swapChainImages_m;
  std::vector<VkImageView> swapChainImageViews_m;

  HveDevice &device_m;
  VkExtent2D windowExtent_m;

  VkSwapchainKHR swapChain_m;
  std::shared_ptr<HveSwapChain> oldSwapChain_m;

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