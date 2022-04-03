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

// hge
#include <hge_components/model_component.hpp>

// std
#include <memory>
#include <vector>
#include <chrono>
#include <unordered_map>

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

    void update(float dt);
    void render(float dt, ModelComponent::map& modelCompMap);

    inline void waitIdle() { vkDeviceWaitIdle(hveDevice_m.device()); }

    inline HveDevice& hveDevice() { return hveDevice_m; }

    inline GLFWwindow* passGLFWwindow() const { return hveWindow_m.getGLFWwindow(); } 
    
  private:
    void init();

    // construct in impl
    HveWindow hveWindow_m;

    HveDevice hveDevice_m {hveWindow_m};
    HveRenderer hveRenderer_m {hveWindow_m, hveDevice_m};

    // shared between multiple system
    std::unique_ptr<HveDescriptorPool> globalPool_m;

    std::vector<std::unique_ptr<HveBuffer>> uboBuffers_m {HveSwapChain::MAX_FRAMES_IN_FLIGHT};
    std::unique_ptr<hnll::HveDescriptorSetLayout> globalSetLayout_m;
    std::vector<VkDescriptorSet> globalDescriptorSets_m {HveSwapChain::MAX_FRAMES_IN_FLIGHT};
    // ptrlize to make it later init 
    std::unique_ptr<SimpleRendererSystem> simpleRendererSystem_m;
    std::unique_ptr<PointLightSystem> pointLightSystem_m;

    HveCamera camera_m{};
    // object for change the camera transform indirectly
    // this object has no model and won't be rendered
    HveGameObject viewerObject_m = HveGameObject::createGameObject();
    KeyboardMovementController cameraController_m {};
};

} // namespace hv