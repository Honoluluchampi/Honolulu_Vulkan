#include <hie.hpp>

// embeded fonts
// download by yourself
#include <roboto_regular.embed>
#include <utility.hpp>

namespace hnll {

Hie::Hie(HveDevice& hveDevice, HveSwapChain& hveSwapChain, GLFWwindow* window) : hveSwapChain_(hveSwapChain)
{  
  setupSpecificVulkanObjects();
  setupImGui(hveDevice, window);
  uploadFonts();
}

Hie::~Hie()
{
  cleanupVulkan();
}

void Hie::setupSpecificVulkanObjects()
{
  createDescriptorPool();
  createRenderPass();
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
  info.ImageCount = hveSwapChain_.imageCount();
  info.CheckVkResultFn = check_vk_result;

  ImGui_ImplVulkan_Init(&info, renderPass_);
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
  ImDrawData* drawData = ImGui::GetDrawData();
  // TODO : if not minimized...
  frameRender(drawData);
  framePresent();
}

void Hie::frameRender(ImDrawData* draw_data)
{
  auto imageAvailableSemaphore = hveSwapChain_.getCurrentImageAvailableSemaphore();
  auto renderFinishedSemaphore = hveSwapChain_.getCurrentRenderFinishedSemaphore();

}

void Hie::framePresent()
{

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

void Hie::createRenderPass()
{
  VkAttachmentDescription attachment = {};
  attachment.format = hveSwapChain_.getSwapChainImageFormat();
  attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment = {};
  color_attachment.attachment = 0;
  color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // implies hve's render pass
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  
  VkRenderPassCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  info.attachmentCount = 1;
  info.pAttachments = &attachment;
  info.subpassCount = 1;
  info.pSubpasses = &subpass;
  info.dependencyCount = 1;
  info.pDependencies = &dependency;
  auto err = vkCreateRenderPass(device_, &info, nullptr, &renderPass_);
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