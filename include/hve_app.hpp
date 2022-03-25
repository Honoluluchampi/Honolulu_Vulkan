#pragma once

#include <hve_window.hpp>
#include <hve_pipeline.hpp>
#include <hve_device.hpp>
#include <hve_swap_chain.hpp>
#include <hve_model.hpp>

// std
#include <memory>
#include <vector>

namespace hve {

class HveApp
{
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    HveApp();
    ~HveApp();

    HveApp(const HveApp &) = delete;
    HveApp &operator= (const HveApp &) = delete;

    void run();
    
  private:
    void loadModels();
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void drawFrame();

    HveWindow hveWindow_m {WIDTH, HEIGHT, "Honolulu Vulkan"};
    HveDevice hveDevice_m {hveWindow_m};
    HveSwapChain hveSwapChain_m {hveDevice_m, hveWindow_m.getExtent()};
    std::unique_ptr<HvePipeline> hvePipeline_m;
    VkPipelineLayout pipelineLayout_m;
    std::vector<VkCommandBuffer> commandBuffers_m;
    std::unique_ptr<HveModel> hveModel_m;
};

} // namespace hv