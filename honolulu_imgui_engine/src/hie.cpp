#include <hie.hpp>

// embeded fonts
// download by yourself
#include <roboto_regular.embed>
#include <utility.hpp>

namespace hnll {

// take s_ptr<HveSwapChain> from hveRenderer
Hie::Hie(HveWindow& hveWindow, HveDevice& hveDevice, s_ptr<HveSwapChain> hveSwapChain)
{  
  setupSpecificVulkanObjects();
  upHieRenderer_ = std::make_unique<HieRenderer>(hveWindow, hveDevice, hveSwapChain);
  setupImGui(hveDevice, hveWindow.getGLFWwindow());
  uploadFonts();
}

Hie::~Hie()
{
  cleanupVulkan();
}

void Hie::setupSpecificVulkanObjects()
{
  createDescriptorPool();
}

void Hie::setupImGui(HveDevice& hveDevice, GLFWwindow* window)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO(); (void)io;
  // enable keyboard control
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // setup ImGui style
  ImGui::StyleColorsDark();

  // setup glfw/vulkan bindings
  ImGui_ImplGlfw_InitForVulkan(window, true);
  ImGui_ImplVulkan_InitInfo info = {};
  info.Instance = hveDevice.instance();
  info.PhysicalDevice = hveDevice.physicalDevice();
  device_ = hveDevice.device();
  info.Device = device_;
  // graphicsFamily's indice is needed (see HveDevice::createCommandPool)
  // but these are never used...
  info.QueueFamily = hveDevice.queueFamilyIndices().graphicsFamily_m.value();
  graphicsQueue_ = hveDevice.graphicsQueue();
  info.Queue = graphicsQueue_;
  info.PipelineCache = VK_NULL_HANDLE;
  info.DescriptorPool = descriptorPool_;
  info.Allocator = nullptr;
  // TODO : make minImageCount consistent with hve
  info.MinImageCount = 2;
  info.ImageCount = upHieRenderer_->hveSwapChain().imageCount();
  info.CheckVkResultFn = check_vk_result;

  // make sure to create render pass before this function
  ImGui_ImplVulkan_Init(&info, upHieRenderer_->getRenderPass());
}

void Hie::cleanupVulkan()
{
  vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
}

void Hie::render()
{
  // start the imgui frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // show the demo window
  ImGui::ShowDemoWindow();

  // render window
  ImGui::Render();

  frameRender();
}

void Hie::frameRender()
{
  // wheather swap chain had been recreated
  if (auto commandBuffer = upHieRenderer_->beginFrame()) {
    upHieRenderer_->beginSwapChainRenderPass(commandBuffer, HIE_RENDER_PASS_ID);
    
    // record the draw data to the command buffer
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    upHieRenderer_->endSwapChainRenderPass(commandBuffer);
  }
}

void Hie::createDescriptorPool()
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
	auto err = vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptorPool_);
	check_vk_result(err);
}

void Hie::uploadFonts()
{
  // load default font 
  ImFontConfig fontConfig;
  fontConfig.FontDataOwnedByAtlas = false;
  ImGuiIO &io = ImGui::GetIO(); (void)io;
  ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
  io.FontDefault = robotoFont;

  // upload fonts
  // Use any command queue
  VkCommandPool commandPool = upHieRenderer_->getCommandPool();
  VkCommandBuffer commandBuffer = upHieRenderer_->getCurrentCommandBuffer();

  auto err = vkResetCommandPool(device_, commandPool, 0);
  check_vk_result(err);
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  err = vkBeginCommandBuffer(commandBuffer, &beginInfo);
  check_vk_result(err);

  ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

  VkSubmitInfo endInfo = {};
  endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  endInfo.commandBufferCount = 1;
  endInfo.pCommandBuffers = &commandBuffer;
  err = vkEndCommandBuffer(commandBuffer);
  check_vk_result(err);
  err = vkQueueSubmit(graphicsQueue_, 1, &endInfo, VK_NULL_HANDLE);
  check_vk_result(err);

  err = vkDeviceWaitIdle(device_);
  check_vk_result(err);
  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

} // namespace hnll