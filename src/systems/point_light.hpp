#pragma once

#include <hve_pipeline.hpp>
#include <hve_device.hpp>
#include <hve_game_object.hpp>
#include <hve_camera.hpp>
#include <hve_frame_info.hpp>

// std
#include <memory>
#include <vector>

namespace hve {

class PointLightSystem
{
  public:

    PointLightSystem(HveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~PointLightSystem();

    PointLightSystem(const PointLightSystem &) = delete;
    PointLightSystem &operator= (const PointLightSystem &) = delete;

    // dont make HveCamera object as a member variable so as to share the camera between multiple render system
    void update(FrameInfo &frameInfo, GlobalUbo &ubo);
    void render(FrameInfo frameInfo);
    
  private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    HveDevice& hveDevice_m;
    std::unique_ptr<HvePipeline> hvePipeline_m;
    VkPipelineLayout pipelineLayout_m;
};

} // namespace hv