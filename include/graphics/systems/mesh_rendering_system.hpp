#pragma once

// hnll
#include <graphics/rendering_system.hpp>
#include <utils/utils.hpp>

// std
#include <vector>

namespace hnll {

class MeshRenderingSystem : public HveRenderingSystem
{
  public:
    MeshRenderingSystem(device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~MeshRenderingSystem();

    // dont make HveCamera object as a member variable so as to share the camera between multiple render system
    void render(FrameInfo frameInfo) override;
    
  private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
    void createPipeline(VkRenderPass renderPass) override;
};

} // namespace hv