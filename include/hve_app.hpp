#pragma once

#include <hve_window.hpp>
#include <hve_pipeline.hpp>

namespace hve {

class HveApp
{
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    void run();
    
  private:
    HveWindow hveWindow{WIDTH, HEIGHT, "Honolulu Vulkan"};
    HvePipeline hvePipeline{"./spv/vert.spv", "./spv/frag.spv"};
};

} // namespace hv