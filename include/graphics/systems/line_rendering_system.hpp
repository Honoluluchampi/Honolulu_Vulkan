#pragma once

// hnll
#include <graphics/rendering_system.hpp>

// lib
#include <vulkan/vulkan.hpp>

// std
#include <vector>

namespace hnll{

class LineRenderingSystem : public HveRenderingSystem
{
  public:
    LineRenderingSystem(device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~LineRenderingSystem();

    void render(FrameInfo frameInfo) override;
    
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    
    // TODO : delete
    static int interpolatingPointsCount;
  private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
    void createPipeline(VkRenderPass renderPass) override;
};

} // namespace hnll