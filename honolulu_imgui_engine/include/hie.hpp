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

// shared with hve (all created by HveDevice)
struct SharedVulkanObjects
{
  VkInstance instance_;
  VkPhysicalDevice physicalDevice_;
  VkDevice device_;
  VkQueue graphicsQueue_;
  VkQueue presentQueue_;

  SharedVulkanObjects(HveDevice& hveDevice);
};

struct SpecificVulkanObjects
{
  HveWindow hveWindow_;
  VkSurfaceKHR surface_; // HveDevice impl
  HveSwapChain hveSwapChain_;
  HveDescriptorPool hveDescriptorPool_;
  VkCommandPool commandPool_; // HveDevice impl

  SpecificVulkanObjects();
};

class Hie
{
public:
  Hie(HveDevice& hveDevice);
  ~Hie();
  Hie(const Hie&) = delete;
  Hie& operator=(const Hie&) = delete;
  Hie(Hie&&) = default;
  Hie& operator=(Hie&&) = default;

  void setupVulkan();
  // All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
  // Your real engine/app may not use them.
  void setupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);

  void cleanupVulkan();
  void cleanupVulkanWindow();

  void frameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
  void framePresent(ImGui_ImplVulkanH_Window* wd);

private:
  static void check_vk_result(VkResult err)
  {
      if (err == 0)
          return;
      fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
      if (err < 0)
          abort();
  }

  static void glfw_error_callback(int error, const char* description)
  {
      fprintf(stderr, "Glfw Error %d: %s\n", error, description);
  }

  VkAllocationCallbacks*   g_Allocator = NULL;
  VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
  VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;

  SharedVulkanObjects sharedVkObjs_;
  SpecificVulkanObjects specificVkObjs_;

  ImGui_ImplVulkanH_Window g_MainWindowData;
  int                      g_MinImageCount = 2;
  bool                     g_SwapChainRebuild = false;
};

} // namespace hnll 