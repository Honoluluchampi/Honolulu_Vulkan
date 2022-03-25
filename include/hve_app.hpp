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
    void recreateSwapChain();
    void recordCommandBuffer(int imageIndex);

    HveWindow hveWindow_m {WIDTH, HEIGHT, "Honolulu Vulkan"};
    HveDevice hveDevice_m {hveWindow_m};
    std::unique_ptr<HveSwapChain> hveSwapChain_m;
    std::unique_ptr<HvePipeline> hvePipeline_m;
    VkPipelineLayout pipelineLayout_m;
    std::vector<VkCommandBuffer> commandBuffers_m;
    std::unique_ptr<HveModel> hveModel_m;
};

} // namespace hv