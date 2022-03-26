#pragma once

#include <hve_window.hpp>
#include <hve_device.hpp>
#include <hve_game_object.hpp>
#include <hve_renderer.hpp>

// std
#include <memory>
#include <vector>

namespace hve {

class HveApp
{
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;
    static constexpr float MAX_FRAME_TIME = 0.05;

    HveApp();
    ~HveApp();

    HveApp(const HveApp &) = delete;
    HveApp &operator= (const HveApp &) = delete;

    void run();
    
  private:
    void loadGameObjects();

    HveWindow hveWindow_m {WIDTH, HEIGHT, "Honolulu Vulkan"};
    HveDevice hveDevice_m {hveWindow_m};
    HveRenderer hveRenderer_m {hveWindow_m, hveDevice_m};

    // game
    std::vector<HveGameObject> gameObjects_m;
};

} // namespace hv