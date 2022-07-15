// hnll
#include <imgui/engine.hpp>

// embeded fonts
// download by yourself
#include <imgui/roboto_regular.embed>
#include <utils/utils.hpp>

namespace hnll {

// take s_ptr<swap_chain> from get_renderer
Hie::Hie(window& hveWindow, device& hveDevice)
  : device_(hveDevice.device())
{  
  setup_specific_vulkan_objects();
  renderer_up_ = std::make_unique<renderer>(hveWindow, hveDevice, false);
  setup_imgui(hveDevice, hveWindow.get_glfw_window());
  upload_font();
}

Hie::~Hie()
{
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  cleanup_vulkan();
}

void Hie::setup_specific_vulkan_objects()
{
  create_descriptor_pool();
}

void Hie::setup_imgui(device& hveDevice, GLFWwindow* window)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO(); (void)io;
  // enable keyboard control
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // setup ImGui style
  ImGui::StyleColorsDark();

  // setup glfw/graphics bindings
  ImGui_ImplGlfw_InitForVulkan(window, true);
  ImGui_ImplVulkan_InitInfo info = {};
  info.Instance = hveDevice.instance();
  info.PhysicalDevice = hveDevice.get_physical_device();
  device_ = hveDevice.device();
  info.Device = device_;
  // graphicsFamily's indice is needed (see device::create_command_pool)
  // but these are never used...
  info.QueueFamily = hveDevice.get_queue_family_indices().graphics_family_.value();
  graphics_queue_ = hveDevice.get_graphics_queue();
  info.Queue = graphics_queue_;
  info.PipelineCache = VK_NULL_HANDLE;
  info.DescriptorPool = descriptor_pool_;
  info.Allocator = nullptr;
  // TODO : make minImageCount consistent with hve
  info.MinImageCount = 2;
  info.ImageCount = renderer_up_->get_swap_chain().get_image_count();
  info.CheckVkResultFn = nullptr;

  // make sure to create render pass before this function
  ImGui_ImplVulkan_Init(&info, renderer_up_->get_render_pass());
}

void Hie::cleanup_vulkan()
{
  vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
}

void Hie::begin_imgui()
{
  // start the imgui frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

// TODO : delete gui demo
void Hie::update(glm::vec3& translation)
{
  // configure position of vase
  translation.x = vec_[0];
  translation.y = vec_[1];
  translation.z = vec_[2];
}

void Hie::render()
{

  // TODO : delete gui demo
  // ImGui::SliderFloat3("translation", vec_, -5, 5);

  // show the demo window
  // ImGui::ShowDemoWindow();

  // render window
  ImGui::Render();

  frame_render();
}

void Hie::frame_render()
{
  // wheather swap chain had been recreated
  if (auto commandBuffer = renderer_up_->begin_frame()) {
    renderer_up_->begin_swap_chain_render_pass(commandBuffer, HIE_RENDER_PASS_ID);
    
    // record the draw data to the command buffer
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    renderer_up_->end_swap_chain_render_pass(commandBuffer);
    renderer_up_->end_frame();
  }
}

void Hie::create_descriptor_pool()
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
	if (vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptor_pool_) != VK_SUCCESS)
    throw std::runtime_error("failed to create descriptor pool.");
}

void Hie::upload_font()
{
  // load default font 
  ImFontConfig fontConfig;
  fontConfig.FontDataOwnedByAtlas = false;
  ImGuiIO &io = ImGui::GetIO(); (void)io;
  ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
  io.FontDefault = robotoFont;

  // upload fonts
  // Use any command queue
  VkCommandPool commandPool = renderer_up_->get_command_pool();
  VkCommandBuffer commandBuffer = renderer_up_->get_current_command_buffer();
  if (vkResetCommandPool(device_, commandPool, 0) != VK_SUCCESS)
    throw std::runtime_error("failed to reset command pool");
  
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    throw std::runtime_error("failed to begin command buffer.");

  ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

  VkSubmitInfo endInfo = {};
  endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  endInfo.commandBufferCount = 1;
  endInfo.pCommandBuffers = &commandBuffer;
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    throw std::runtime_error("failed to end command buffer.");
  
  if (vkQueueSubmit(graphics_queue_, 1, &endInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    throw std::runtime_error("failed to submit font upload queue.");

  vkDeviceWaitIdle(device_);
  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

} // namespace hnll