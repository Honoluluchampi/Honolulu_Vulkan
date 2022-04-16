#include <hie.hpp>

namespace hnll {

Hie::Hie()
{
  // debug window creation
  window_m = glfwCreateWindow(1280, 720, "imgui window", NULL, NULL);
  
  setupVulkan();
  // seupVulkanWindow();
}

Hie::~Hie()
{
  cleanupVulkanWindow();
  cleanupVulkan();

  glfwDestroyWindow(window_m);
}

void Hie::setupVulkan()
{

}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
void Hie::setupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
{

}

void Hie::cleanupVulkan()
{

}

void Hie::cleanupVulkanWindow()
{

}

void Hie::frameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{

}

void Hie::framePresent(ImGui_ImplVulkanH_Window* wd)
{

}

} // namespace hnll