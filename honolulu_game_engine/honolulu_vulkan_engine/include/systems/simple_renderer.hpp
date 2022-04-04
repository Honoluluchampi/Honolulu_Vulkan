#pragma once

#include <hve_rendering_system.hpp>
#include <hve_camera.hpp>
#include <hge_components/model_component.hpp>

// std
#include <vector>

namespace hnll {

class SimpleRendererSystem : public HveRenderingSystem<ModelComponent>
{
  public:
    SimpleRendererSystem(HveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~SimpleRendererSystem();

    // dont make HveCamera object as a member variable so as to share the camera between multiple render system
    void render(FrameInfo frameInfo) override;
    
  private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
    void createPipeline(VkRenderPass renderPass) override;
};

} // namespace hv