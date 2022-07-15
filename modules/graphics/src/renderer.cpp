// hnll
#include <graphics/renderer.hpp>

//std
#include <stdexcept>
#include <array>

namespace hnll {

// static members
uint32_t renderer::current_image_index_ = 0;
int renderer::current_frame_index_ = 0;
bool renderer::swap_chain_recreated_ = false;
std::vector<VkCommandBuffer> renderer::submitting_command_buffers_ = {};
u_ptr<swap_chain> renderer::swap_chain_ = nullptr;

renderer::renderer(window& window, device& device, bool recreateFromScratch)
 : window_{window}, device_{device}
{
  // recreate swap chain dependent objects
  if (recreateFromScratch) recreate_swap_chain();
  create_command_buffers();
}

renderer::~renderer()
{
  free_command_buffers();
}

void renderer::recreate_swap_chain()
{
  // stop calculation until the window is minimized
  auto extent = window_.get_extent();
  while (extent.width == 0 || extent.height == 0) {
    extent = window_.get_extent();
    glfwWaitEvents();
  }
  // wait for finishing the current task
  vkDeviceWaitIdle(device_.device());

  // for first creation
  if (swap_chain_ == nullptr)
    swap_chain_ = std::make_unique<swap_chain>(device_, extent);
  // recreate
  else {
    // move the ownership of the current swap chain to old one.
    std::unique_ptr<swap_chain> oldSwapChain = std::move(swap_chain_);
    swap_chain_ = std::make_unique<swap_chain>(device_, extent, std::move(oldSwapChain));

  // TODO : enabled this segmetn
    // if (!oldSwapChain->compare_swap_chain_formats(*swap_chain_.get()))
    //   throw std::runtime_error("swap chian image( or depth) format has chainged");

    // command buffers no longer depend on the swap chain image count
  }
  // if render pass compatible, do nothing else 

  // execute this function at the last of derived function's recreate_swap_chain();
  if (next_renderer_) next_renderer_->recreate_swap_chain();

  swap_chain_recreated_ = true;
}

void renderer::reset_frame()
{
  swap_chain_recreated_ = false;
  submitting_command_buffers_.clear();
  // increment current_frame_index_
  if (++current_frame_index_ == swap_chain::MAX_FRAMES_IN_FLIGHT)
    current_frame_index_ = 0;
}

void renderer::create_command_buffers() 
{
  // 2 or 3
  command_buffers_.resize(swap_chain::MAX_FRAMES_IN_FLIGHT);

  // specify command pool and number of buffers to allocate
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  // if the allocated command buffers are primary or secondary command buffers
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = device_.get_command_pool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());
  
  if (vkAllocateCommandBuffers(device_.device(), &allocInfo, command_buffers_.data()) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate command buffers!");
}

void renderer::free_command_buffers()
{
  vkFreeCommandBuffers(
      device_.device(), 
      device_.get_command_pool(), 
      static_cast<float>(command_buffers_.size()), 
      command_buffers_.data());
  command_buffers_.clear();
}

void renderer::cleanup_swap_chain()
{
  swap_chain_.reset();
}

VkCommandBuffer renderer::begin_frame()
{
  assert(!is_frame_started_ && "Can't call begin_frame() while already in progress");
  // get finished image from swap chain
  // TODO : get isFirstRenderer()
  if (!is_last_renderer()) {
    reset_frame();
    auto result = swap_chain_->acquire_next_image(&current_image_index_);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_.is_resized()) {
      window_.reset_window_resized_flag();
      recreate_swap_chain();
      // the frame has not successfully started
      return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      throw std::runtime_error("failed to acquire swap chain image!");
  }

  is_frame_started_ = true;

  auto commandBuffer = get_current_command_buffer();
  // assert(commandBuffer != VK_NULL_HANDLE && "");
    // start reconding command buffers
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // // how to use the command buffer
  // beginInfo.flags = 0;
  // // state to inherit from the calling primary command buffers
  // // (only relevant for secondary command buffers)
  // beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
    throw std::runtime_error("failed to begin recording command buffer!");
  return commandBuffer;
}

void renderer::end_frame()
{
  assert(is_frame_started_ && "Can't call end_frame() while the frame is not in progress");
  auto commandBuffer = get_current_command_buffer();
  // end recording command buffer
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
    throw std::runtime_error("failed to record command buffer!");

#ifdef IMGUI_DISABLED
  auto result = swap_chain_->submit_command_buffers(&commandBuffer, &current_image_index_);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_.is_resized()) {
    window_.reset_window_resized_flag();
    recreate_swap_chain();
  }
  else if (result != VK_SUCCESS)
    throw std::runtime_error("failed to present swap chain image!");

  is_frame_started_ = false;
  // increment current_frame_index_
  if (++current_frame_index_ == swap_chain::MAX_FRAMES_IN_FLIGHT)
    current_frame_index_ = 0;
#else
  submitting_command_buffers_.push_back(commandBuffer);
#endif

  is_frame_started_ = false;
  
  if (is_last_renderer()) {
    submit_command_buffers();
  }
}

void renderer::begin_swap_chain_render_pass(VkCommandBuffer commandBuffer, int renderPassId)
{
  assert(is_frame_started_ && "Can't call begin_swap_chain_render_pass() while the frame is not in progress.");
  assert(commandBuffer == get_current_command_buffer() && "Can't beginig render pass on command buffer from a different frame.");

  // starting a render pass
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

#ifdef IMGUI_DISABLED
  renderPassInfo.renderPass = swap_chain_->get_render_pass();
  renderPassInfo.framebuffer = swap_chain_->getFrameBuffer(current_image_index_);
#else
  renderPassInfo.renderPass = swap_chain_->get_render_pass(renderPassId);
  renderPassInfo.framebuffer = swap_chain_->get_frame_buffer(renderPassId, current_image_index_);
#endif  

  // the pixels outside this region will have undefined values
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = swap_chain_->get_swap_chain_extent();

  // default value for color and depth
  std::array<VkClearValue, 2> clearValues;
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  // last parameter controls how the drawing commands within the render pass
  // will be provided. 
  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  // dynamic viewport and scissor
  // transformation of the image                            
  // draw entire framebuffer
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(swap_chain_->get_swap_chain_extent().width);
  viewport.height = static_cast<float>(swap_chain_->get_swap_chain_extent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  // cut the region of the framebuffer(swap chain)
  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swap_chain_->get_swap_chain_extent();
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void renderer::end_swap_chain_render_pass(VkCommandBuffer commandBuffer)
{
  assert(is_frame_started_ && "Can't call end_swap_chain_render_pass() while the frame is not in progress.");
  assert(commandBuffer == get_current_command_buffer() && "Can't ending render pass on command buffer from a different frame.");

  // finish render pass and recording the comand buffer
  vkCmdEndRenderPass(commandBuffer);
}

#ifndef IMGUI_DISABLED
void renderer::submit_command_buffers()
{
  auto result = swap_chain_->submit_command_buffers(submitting_command_buffers_.data(), &current_image_index_);
  if (result != VK_ERROR_OUT_OF_DATE_KHR && result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS) 
    throw std::runtime_error("failed to present swap chain image!");  
}
#endif

} // namespace hve