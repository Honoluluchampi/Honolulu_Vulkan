#pragma once

#include <hve_window.hpp>

namespace hve {

class HveApp
{
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    void run();
    
  private:
    HveWindow hveWindow{WIDTH, HEIGHT, "Honolulu Vulkan"};
};

} // namespace hv