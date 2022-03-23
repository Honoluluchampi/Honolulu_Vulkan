#pragma once

#include <hve_window.hpp>

namespace hve {

class HvApp
{
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    void run() {}
    
  private:
    HvWindow hvWindow{WIDTH, HEIGHT, "Honolulu Vulkan"};
};

} // namespace hv