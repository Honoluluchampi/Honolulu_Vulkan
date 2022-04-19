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

// SpecificVulkanObjects::SpecificVulkanObjects() : hveWindow_(1280, 720, "hie window")
// {

// }

Hie::Hie(HveDevice& hveDevice, GLFWwindow* window) : sharedVkObjs_(hveDevice)
{  
  setupImGui(hveDevice, window);
  setupVulkan();
  // seupVulkanWindow();
}

Hie::~Hie()
{
  cleanupVulkanWindow();
  cleanupVulkan();

}

void Hie::setupImGui(HveDevice& hveDevice, GLFWwindow* window)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  // setup glfw/vulkan bindings
  ImGui_ImplGlfw_InitForVulkan(window, false);
  ImGui_ImplVulkan_InitInfo info;
  info.Instance = hveDevice.instance();
  info.PhysicalDevice = hveDevice.physicalDevice();
  device_ = hveDevice.device();
  info.Device = device_;
  // graphicsFamily's indice is needed (see HveDevice::createCommandPool)
  info.QueueFamily = hveDevice.queueFamilyIndices().graphicsFamily_m.value();
  info.Queue = hveDevice.graphicsQueue();
  // TODO : search this
  info.PipelineCache = VK_NULL_HANDLE;
  info.DescriptorPool = createDescriptorPool(info.Device);
  ImGui_ImplVulkan_Init(&info, );
  // setup ImGui style
  ImGui::StyleColorsDark();
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
  vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
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

VkDescriptorPool Hie::createDescriptorPool()
{
  // Create Descriptor Pool
  // TODO : understand these parameters
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	err = vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptorPool_);
	check_vk_result(err);
}

} // namespace hnll