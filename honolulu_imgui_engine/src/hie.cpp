#include <hie.hpp>

namespace hnll {

SharedVulkanObjects::SharedVulkanObjects(HveDevice& hd)
{
  instance_ = hd.instance();
  physicalDevice_ = hd.physicalDevice();
  device_ = hd.device();
  graphicsQueue_ = hd.graphicsQueue();
  presentQueue_ = hd.presentQueue();
}

SpecificVulkanObjects::SpecificVulkanObjects() : hveWindow_(1280, 720, "hie window")
{

}

Hie::Hie(HveDevice& hveDevice) : sharedVkObjs_(hveDevice)
{  
  setupVulkan();
  // seupVulkanWindow();
}

Hie::~Hie()
{
  cleanupVulkanWindow();
  cleanupVulkan();

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