#include <hie_renderer.hpp>

namespace hnll {

HieRenderer::HieRenderer(HveWindow& window, HveDevice& hveDevice, HveSwapChain& hveSwapChain) : 
  HveRenderer(window, hveDevice)
{
  isLastRenderer();
}

void HieRenderer::recreateSwapChainDependencies()
{
  hveSwapChain_m->setRenderPass(createRenderPass(), HIE_RENDER_PASS_ID);
  hveSwapChain_m->setFramebuffers(createFramebuffers(), HIE_RENDER_PASS_ID);
}

VkRenderPass HieRenderer::createRenderPass()
{
  VkAttachmentDescription attachment = {};
  attachment.format = hveSwapChain_m->getSwapChainImageFormat();
  attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // hve render pass's final layout
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

  VkRenderPass renderPass;
  
  if (vkCreateRenderPass(hveDevice_m.device(), &info, nullptr, &renderPass) != VK_SUCCESS)
    throw std::runtime_error("failed to create render pass.");

  return renderPass;
}

std::vector<VkFramebuffer> HieRenderer::createFramebuffers()
{
  // imgui frame buffer only takes image view attachment 
  VkImageView attachment;

  VkFramebufferCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  // make sure to create renderpass before frame buffers
  info.renderPass = hveSwapChain_m->getRenderPass(HIE_RENDER_PASS_ID);
  info.attachmentCount = 1;
  info.pAttachments = &attachment;
  auto extent = hveSwapChain_m->getSwapChainExtent();
  info.width = extent.width;
  info.height = extent.height;
  info.layers = 1;

  // as many as image view count
  auto imageCount = hveSwapChain_m->imageCount();
  std::vector<VkFramebuffer> framebuffers(imageCount);
  for (size_t i = 0; i < imageCount; i++) {
    attachment = hveSwapChain_m->getImageView(i);
    if (vkCreateFramebuffer(hveDevice_m.device(), &info, nullptr, &framebuffers[i]) != VK_SUCCESS)
      throw std::runtime_error("failed to create frame buffer.");
  }

  return framebuffers;
}

} // namespace hnll