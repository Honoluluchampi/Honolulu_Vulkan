#pragma once

#include <graphics/rendering_system.hpp>

// std
#include <vector>

namespace hnll {

class PointLightSystem : public HveRenderingSystem
{
  public:
    PointLightSystem(HveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~PointLightSystem();

    PointLightSystem(const PointLightSystem &) = delete;
    PointLightSystem &operator= (const PointLightSystem &) = delete;

    // dont make HveCamera object as a member variable so as to share the camera between multiple render system
    void render(FrameInfo frameInfo) override;
    
  private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
    void createPipeline(VkRenderPass renderPass) override;
};

} // namespace hv