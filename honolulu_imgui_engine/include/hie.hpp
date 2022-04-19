#pragma once

// basic header
#include <imgui.h>

// api-specific  header
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

// lib
#include <GLFW/glfw3.h>

// hve
#include <hve.hpp>

// std
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort

// debug
#ifndef NDEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

// shared objects
// Vulkan Instance
// Vulkan Physical Device
// Vulkan Logical Device
// Renderer Queue
// Present Queue

// imgui-specific objects
// window (config window)
// surface
// swap chain
// depth image
// render pass
// frame buffer
// descriptor pool
// semaphores
// command pool
// command buffer

namespace hnll {

class Hie
{
public:
  Hie(HveDevice& hveDevice, HveSwapChain& hveSwapChain, GLFWwindow* window);
  ~Hie();
  Hie(const Hie&) = delete;
  Hie& operator=(const Hie&) = delete;
  Hie(Hie&&) = default;
  Hie& operator=(Hie&&) = default;

  void frameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
  void framePresent(ImGui_ImplVulkanH_Window* wd);

private:
  // set up ImGui context
  void setupImGui(HveDevice& hveDevice, HveSwapChain& hveSwapChain, GLFWwindow* window);
  // share the basic vulkan object with hve, so there is nothing to do for now
  void setupSpecificVulkanObjects(HveSwapChain& hveSwapChain);
  void uploadFonts(HveDevice& hveDevice);
  void cleanupVulkan();
  void createDescriptorPool();
  void createRenderPass(HveSwapChain& hveSwapChain);

  static void check_vk_result(VkResult err)
  {
      if (err == 0)
          return;
      fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
      if (err < 0)
          abort();
  }

  static void glfw_error_callback(int error, const char* description)
  { fprintf(stderr, "Glfw Error %d: %s\n", error, description); }

  VkDevice device_;
  VkDescriptorPool descriptorPool_;
  VkRenderPass renderPass_;
  VkQueue graphicsQueue_;

  ImGui_ImplVulkanH_Window mainWindowData_;
  // TODO : make it consistent with hve
  int minImageCount_ = 2;
  bool swapChainRebuild_ = false;
  bool isHieRunning_ = false;
};

} // namespace hnll 