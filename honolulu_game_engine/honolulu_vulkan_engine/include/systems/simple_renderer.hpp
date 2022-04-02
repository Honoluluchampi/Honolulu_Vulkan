#pragma once

#include <hve_pipeline.hpp>
#include <hve_device.hpp>
#include <hve_game_object.hpp>
#include <hve_camera.hpp>
#include <hve_frame_info.hpp>

// std
#include <memory>
#include <vector>

namespace hnll {

class SimpleRendererSystem
{
  public:

    SimpleRendererSystem(HveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~SimpleRendererSystem();

    SimpleRendererSystem(const SimpleRendererSystem &) = delete;
    SimpleRendererSystem &operator= (const SimpleRendererSystem &) = delete;
    SimpleRendererSystem(SimpleRendererSystem &&) = default;
    SimpleRendererSystem &operator= (SimpleRendererSystem &&) = default;

    // dont make HveCamera object as a member variable so as to share the camera between multiple render system
    void render(FrameInfo frameInfo);
    
  private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    HveDevice& hveDevice_m;
    std::unique_ptr<HvePipeline> hvePipeline_m;
    VkPipelineLayout pipelineLayout_m;
};

} // namespace hv