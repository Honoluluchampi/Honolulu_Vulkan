#include "hve_swap_chain.hpp"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

// this should be configurable
#define IMGUI_ENABLED;

namespace hnll {

HveSwapChain::HveSwapChain(HveDevice &deviceRef, VkExtent2D extent)
    : device_m{deviceRef}, windowExtent_m{extent} 
{
  init();
}

HveSwapChain::HveSwapChain(HveDevice &deviceRef, VkExtent2D extent, std::shared_ptr<HveSwapChain> previous)
    : device_m{deviceRef}, windowExtent_m{extent}, oldSwapChain_m(previous) 
{
  init();
  // clean up old swap chain since it's no longer neeeded
  oldSwapChain_m.reset();
}

void HveSwapChain::init()
{
  createSwapChain();
  createImageViews();

#ifdef __IMGUI_DISABLED
  renderPass_m = createRenderPass();
  createDepthResources();
  swapChainFramebuffers_m = createFramebuffers(renderPass_m);
#else
  // only create hve's renderPass
  createMultipleRenderPass();
  createDepthResources();
  createMultipleFramebuffers();
#endif

  createSyncObjects();
}

HveSwapChain::~HveSwapChain() 
{
  for (auto imageView : swapChainImageViews_m) {
    vkDestroyImageView(device_m.device(), imageView, nullptr);
  }
  swapChainImageViews_m.clear();

  if (swapChain_m != nullptr) {
    vkDestroySwapchainKHR(device_m.device(), swapChain_m, nullptr);
    swapChain_m = nullptr;
  }

  for (int i = 0; i < depthImages_m.size(); i++) {
    vkDestroyImageView(device_m.device(), depthImageViews_m[i], nullptr);
    vkDestroyImage(device_m.device(), depthImages_m[i], nullptr);
    vkFreeMemory(device_m.device(), depthImageMemorys_m[i], nullptr);
  }

#ifdef __IMGUI_DISABLED
  for (auto framebuffer : swapChainFramebuffers_m) {
    vkDestroyFramebuffer(device_m.device(), framebuffer, nullptr);
  }
  vkDestroyRenderPass(device_m.device(), renderPass_m, nullptr);
#else
  for (auto framebuffers : multipleFramebuffers_m) {
    for (auto framebuffer : framebuffers) {
      vkDestroyFramebuffer(device_m.device(), framebuffer, nullptr);
    }
  }
  for (auto renderPass : multipleRenderPass_m)
    vkDestroyRenderPass(device_m.device(), renderPass, nullptr);
#endif

  // cleanup synchronization objects
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(device_m.device(), renderFinishedSemaphores_m[i], nullptr);
    vkDestroySemaphore(device_m.device(), imageAvailableSemaphores_m[i], nullptr);
    vkDestroyFence(device_m.device(), inFlightFences_m[i], nullptr);
  }
}

VkResult HveSwapChain::acquireNextImage(uint32_t *imageIndex) 
{
  vkWaitForFences(device_m.device(), 1, &inFlightFences_m[currentFrame_m], 
      VK_TRUE, std::numeric_limits<uint64_t>::max());

  VkResult result = vkAcquireNextImageKHR(
      device_m.device(),
      swapChain_m,
      std::numeric_limits<uint64_t>::max(),
      imageAvailableSemaphores_m[currentFrame_m],  // must be a not signaled semaphore
      VK_NULL_HANDLE,
      imageIndex);

  return result;
}

VkResult HveSwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) 
{
  // specify a timeout in nanoseconds for an image
  auto timeout = UINT64_MAX;
  // check if a previous frame is using this image
  if(imagesInFlight_m[*imageIndex] != VK_NULL_HANDLE)
      vkWaitForFences(device_m.device(), 1, &imagesInFlight_m[*imageIndex], VK_TRUE, timeout);
  // mark the image as now being in use by this frame
  imagesInFlight_m[*imageIndex] = inFlightFences_m[currentFrame_m];

  //submitting the command buffer
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  // the index of waitSemaphores corresponds to the index of waitStages
  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores_m[currentFrame_m]};
  // which stage of the pipeline to wait
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  // which command buffers to actually submit for execution
  // should submit the command buffer that binds the swap chain image 
  // we just acquired as color attachiment.

  // TODO : configure renderer count in a systematic way
#ifdef __IMGUI_DISABLED
  submitInfo.commandBufferCount = 1;
#else
  submitInfo.commandBufferCount = 2;
#endif
  submitInfo.pCommandBuffers = buffers;
  // specify which semaphores to signal once the comand buffer have finished execution
  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores_m[currentFrame_m]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  // need to be manually restore the fence to the unsignaled state
  vkResetFences(device_m.device(), 1, &inFlightFences_m[currentFrame_m]);

  // submit the command buffer to the graphics queue with fence
  if (vkQueueSubmit(device_m.graphicsQueue(), 1, &submitInfo, inFlightFences_m[currentFrame_m]) != VK_SUCCESS)
      throw std::runtime_error("failed to submit draw command buffer!");

  // configure subpass dependencies in VkRenderPassFacotry::createRenderPass

  // presentation
  // submit the result back to the swap chain to have it eventually show up on the screen
  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain_m};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = imageIndex;
  // nesessary for multi swap chain
  // presentInfo.pResults = nullptr;

  auto result = vkQueuePresentKHR(device_m.presentQueue(), &presentInfo);

  currentFrame_m = (currentFrame_m + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

void HveSwapChain::createSwapChain() 
{
  SwapChainSupportDetails swapChainSupport = device_m.getSwapChainSupport();

  auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats_m);
  auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes_m);
  auto extent = chooseSwapExtent(swapChainSupport.capabilities_m);
  // how many images id like to have in the swap chain
  uint32_t imageCount = swapChainSupport.capabilities_m.minImageCount + 1;
  // make sure to not exceed the maximum number of images
  if (swapChainSupport.capabilities_m.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities_m.maxImageCount) {
    imageCount = swapChainSupport.capabilities_m.maxImageCount;
  }
  // fill in a structure with required information
  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = device_m.surface();

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  // the amount of layers each image consists of
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // specify how to handle swap chain images by multiple queue families
  QueueFamilyIndices indices = device_m.findPhysicalQueueFamilies();
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily_m.value(), indices.presentFamily_m.value()};

  if (indices.graphicsFamily_m.value() != indices.presentFamily_m.value()) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    // this option offers best performance
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;      // Optional
    createInfo.pQueueFamilyIndices = nullptr;  // Optional
  }

  // like a 90 degree clockwise rotation or horizontal flip
  createInfo.preTransform = swapChainSupport.capabilities_m.currentTransform;
  // ignore alpha channel
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  // ignore the color of obscured pixels
  createInfo.clipped = VK_TRUE;
  // invalid or unoptimized swap chain should be reacreated from scratch
  createInfo.oldSwapchain = oldSwapChain_m == nullptr ? VK_NULL_HANDLE : oldSwapChain_m->swapChain_m;

  // create swap chain
  if (vkCreateSwapchainKHR(device_m.device(), &createInfo, nullptr, &swapChain_m) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  // we only specified a minimum number of images in the swap chain, so the implementation is
  // allowed to create a swap chain with more. That's why we'll first query the final number of
  // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
  // retrieve the handles.
  vkGetSwapchainImagesKHR(device_m.device(), swapChain_m, &imageCount, nullptr);
  swapChainImages_m.resize(imageCount);
  vkGetSwapchainImagesKHR(device_m.device(), swapChain_m, &imageCount, swapChainImages_m.data());

  swapChainImageFormat_m = surfaceFormat.format;
  swapChainExtent_m = extent;
}

void HveSwapChain::createImageViews() 
{
  // create image view for all VkImage in the swap chain
  swapChainImageViews_m.resize(swapChainImages_m.size());
  for (size_t i = 0; i < swapChainImages_m.size(); i++) {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = swapChainImages_m[i];
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = swapChainImageFormat_m;
      // swizzle the color channeld around
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      // subresourceRange describes what images purpose is 
      // and which part of the image should be accessed
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;
      if (vkCreateImageView(device_m.device(), &createInfo, nullptr,
          &swapChainImageViews_m[i]) != VK_SUCCESS) {
          throw std::runtime_error("failed to create image views!");
      }
  }
}

VkRenderPass HveSwapChain::createRenderPass() 
{
  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = findDepthFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = getSwapChainImageFormat();
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  // what to do with the data in the attachment before and after rendering
  // clear the framebuffer to black before drawing a new frame
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

#ifdef __IMGUI_DISABLED
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
#else
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
#endif

  VkAttachmentReference colorAttachmentRef = {};
  // which attachment to reference by its index
  colorAttachmentRef.attachment = 0;
  // which layout we would like the attachment to have during a subpass
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // subpass creation
  // vulkan may support compute subpasses in the future
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency = {};
  // the implicit subpass befor or after the render pass
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.srcAccessMask = 0;
  // the operations to wait on and the stages in which these operations occur
  dependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  // dstSubpass souhld be higher than srcSubpass to prevent cycles
  dependency.dstSubpass = 0;
  dependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  // attachment and subpass can be array of those;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  VkRenderPass renderPass;

  if (vkCreateRenderPass(device_m.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }

#ifdef __IMGUI_DISABLED
  renderPass_m = renderPass;
#endif

  return renderPass;
}

std::vector<VkFramebuffer> HveSwapChain::createFramebuffers(VkRenderPass renderPass) 
{
  std::vector<VkFramebuffer> swapChainFramebuffers(imageCount());
  for (size_t i = 0; i < imageCount(); i++) {
    std::array<VkImageView, 2> attachments = {swapChainImageViews_m[i], depthImageViews_m[i]};

    VkExtent2D swapChainExtent = getSwapChainExtent();
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    // renderPass the framebuffer needs to be compatible
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device_m.device(), &framebufferInfo, nullptr,
            &swapChainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void HveSwapChain::createMultipleRenderPass()
{
  multipleRenderPass_m[HVE_RENDER_PASS_ID] = createRenderPass();
}

void HveSwapChain::createMultipleFramebuffers()
{
  multipleFramebuffers_m[HVE_RENDER_PASS_ID] = createFramebuffers(multipleRenderPass_m[0]);
}

void HveSwapChain::createDepthResources() 
{
  swapChainDepthFormat_m = findDepthFormat();
  VkExtent2D swapChainExtent = getSwapChainExtent();

  depthImages_m.resize(imageCount());
  depthImageMemorys_m.resize(imageCount());
  depthImageViews_m.resize(imageCount());

  for (int i = 0; i < depthImages_m.size(); i++) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = swapChainDepthFormat_m;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    device_m.createImageWithInfo(
        imageInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImages_m[i],
        depthImageMemorys_m[i]);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = depthImages_m[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = swapChainDepthFormat_m;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device_m.device(), &viewInfo, nullptr, &depthImageViews_m[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }
  }
}

void HveSwapChain::createSyncObjects() 
{
  imageAvailableSemaphores_m.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores_m.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences_m.resize(MAX_FRAMES_IN_FLIGHT);
  // initially not a single framce is using an image, so initialize it to no fence
  imagesInFlight_m.resize(imageCount(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  // initialize fences in the signaled state as if they had been rendered an initial frame
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    // future version of the vulkan api may add functionality for other parameters
    if (vkCreateSemaphore(device_m.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores_m[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device_m.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores_m[i]) != VK_SUCCESS ||
        vkCreateFence(device_m.device(), &fenceInfo, nullptr, &inFlightFences_m[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
  }
}

VkSurfaceFormatKHR HveSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) 
{
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR HveSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) 
{
  // animation becomes storange if use present mode : mailbox

  for (const auto &availablePresentMode : availablePresentModes) {
    // prefer triple buffering
    // high cost
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      // std::cout << "Present mode: Mailbox" << std::endl;
      return availablePresentMode;
    }
  }

  // for (const auto &availablePresentMode : availablePresentModes) {
  //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
  //     std::cout << "Present mode: Immediate" << std::endl;
  //     return availablePresentMode;
  //   }
  // }

  std::cout << "Present mode: V-Sync" << std::endl;
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HveSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) 
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent = windowExtent_m;
    actualExtent.width = std::max(capabilities.minImageExtent.width,
                         std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height,
                          std::min(capabilities.maxImageExtent.height, actualExtent.height));
    return actualExtent;
  }
}

VkFormat HveSwapChain::findDepthFormat() 
{
  return device_m.findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

}  // namespace lve