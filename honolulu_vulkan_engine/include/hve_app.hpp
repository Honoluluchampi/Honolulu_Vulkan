#pragma once

#include <hve_window.hpp>
#include <hve_device.hpp>
#include <hve_game_object.hpp>
#include <hve_renderer.hpp>
#include <hve_descriptor_set_layout.hpp>
#include <systems/simple_renderer.hpp>
#include <systems/point_light.hpp>
#include <hve_camera.hpp>
#include <keyboard_movement_controller.hpp>
#include <hve_buffer.hpp>

// std
#include <memory>
#include <vector>

namespace hnll {

class Hve
{
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;
    static constexpr float MAX_FRAME_TIME = 0.05;

    Hve(const char* windowName = "honolulu engine");
    ~Hve();

    Hve(const Hve &) = delete;
    Hve &operator= (const Hve &) = delete;

    void run();
    
  private:
    void init();
    void loadGameObjects();

    // construct in impl
    HveWindow hveWindow_m;

    HveDevice hveDevice_m {hveWindow_m};
    HveRenderer hveRenderer_m {hveWindow_m, hveDevice_m};

    // shared between multiple system
    std::unique_ptr<HveDescriptorPool> globalPool_m;
    // game
    HveGameObject::Map gameObjects_m;

    std::vector<std::unique_ptr<HveBuffer>> uboBuffers {HveSwapChain::MAX_FRAMES_IN_FLIGHT};
    std::unique_ptr<hnll::HveDescriptorSetLayout> globalSetLayout;
    std::vector<VkDescriptorSet> globalDescriptorSets {HveSwapChain::MAX_FRAMES_IN_FLIGHT};
    // ptrlize to make it later init 
    std::unique_ptr<SimpleRendererSystem> simpleRendererSystem;
    std::unique_ptr<PointLightSystem> pointLightSystem;

    HveCamera camera{};
    // object for change the camera transform indirectly
    // this object has no model and won't be rendered
    HveGameObject viewerObject = HveGameObject::createGameObject();
    KeyboardMovementController cameraController{};
};

} // namespace hv