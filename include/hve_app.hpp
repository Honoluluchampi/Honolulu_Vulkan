#pragma once

#include <hve_window.hpp>
#include <hve_pipeline.hpp>
#include <hve_device.hpp>
#include <hve_swap_chain.hpp>

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
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void drawFrame();

    HveWindow hveWindow_m{WIDTH, HEIGHT, "Honolulu Vulkan"};
    HveDevice hveDevice_m{hveWindow_m};
    HveSwapChain hveSwapChain{hveDevice_m, hveWindow_m.getExtent()};
    std::unique_ptr<HvePipeline> hvePipeline_m;
    VkPipelineLayout pipelineLayout_m;
    std::vector<VkCommandBuffer> commandBuffers;
};

} // namespace hv