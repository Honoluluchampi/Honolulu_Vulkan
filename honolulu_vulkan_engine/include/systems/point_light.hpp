#pragma once

#include <hve_rendering_system.hpp>
#include <hge_components/point_light_component.hpp>

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
    void update(FrameInfo &frameInfo, GlobalUbo &ubo) override;
    void render(FrameInfo frameInfo) override;
    
  private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
    void createPipeline(VkRenderPass renderPass) override;
};

} // namespace hv