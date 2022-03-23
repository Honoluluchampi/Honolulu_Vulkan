#pragma once

#include <hve_window.hpp>
#include <hve_pipeline.hpp>
#include <hve_device.hpp>

namespace hve {

class HveApp
{
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    void run();
    
  private:
    HveWindow hveWindow_m{WIDTH, HEIGHT, "Honolulu Vulkan"};
    HveDevice hveDevice_m{hveWindow_m};
    HvePipeline hvePipeline_m{
      hveDevice_m,
      "./shader/spv/vert.spv", 
      "./shader/spv/frag.spv",
      HvePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
};

} // namespace hv