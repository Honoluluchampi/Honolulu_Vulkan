#include <hve_rendering_system.hpp>

namespace hnll {

HveRenderingSystem::HveRenderingSystem
  (HveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : hveDevice_m(device)
{
  // copy and paste below programs in derived class
  // createPipelineLayout(globalSetLayout);
  // createPipeline(renderPass);
}

HveRenderingSystem::~HveRenderingSystem()
{
  vkDestroyPipelineLayout(hveDevice_m.device(), pipelineLayout_m, nullptr);
}

} // namespace hnll