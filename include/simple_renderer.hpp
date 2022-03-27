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

class SimpleRendererSystem
{
  public:

    SimpleRendererSystem(HveDevice& device, VkRenderPass renderPass);
    ~SimpleRendererSystem();

    SimpleRendererSystem(const SimpleRendererSystem &) = delete;
    SimpleRendererSystem &operator= (const SimpleRendererSystem &) = delete;

    // dont make HveCamera object as a member variable so as to share the camera between multiple render system
    void renderGameObjects(FrameInfo frameInfo, std::vector<HveGameObject> &gameObjects);
    
  private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);

    HveDevice& hveDevice_m;
    std::unique_ptr<HvePipeline> hvePipeline_m;
    VkPipelineLayout pipelineLayout_m;
};

} // namespace hv