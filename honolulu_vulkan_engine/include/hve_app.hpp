#pragma once

#include <hve_window.hpp>
#include <hve_device.hpp>
#include <hve_renderer.hpp>
#include <hve_descriptor_set_layout.hpp>
#include <hve_camera.hpp>
#include <hve_buffer.hpp>
#include <hve_rendering_system.hpp>

// hge
#include <hge_components/viewer_component.hpp>

// std
#include <memory>
#include <vector>
#include <chrono>
#include <stdexcept>
#include <unordered_map>

namespace hnll {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

class Hve
{
  using map = std::unordered_map<RenderType, std::unique_ptr<HveRenderingSystem>>;

  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;
    static constexpr float MAX_FRAME_TIME = 0.05;

    Hve(const char* windowName = "honolulu engine");
    ~Hve();

    Hve(const Hve &) = delete;
    Hve &operator= (const Hve &) = delete;

    void render(ViewerComponent& viewerComp);

    // takes s_ptr<RenderableComponent>
    template<class RC>
    void addRenderableComponent(RC&& target)
    { renderingSystems_m[target->getRenderType()]->addRenderTarget(target->getCompId(), std::forward<RC>(target)); }
    
    template<class RC>
    void replaceRenderableComponent(RC&& target)
    { renderingSystems_m[target->getRenderType()]->replaceRenderTarget(target->getCompId(), std::forward<RC>(target)); }

    void removeRenderableComponentWithoutOwner(RenderType type, HgeComponent::compId id);

    inline void waitIdle() { vkDeviceWaitIdle(hveDevice_m.device()); }

    inline HveDevice& hveDevice() { return hveDevice_m; }
    inline HveRenderer& hveRenderer() { return hveRenderer_m; }
    inline HveSwapChain& hveSwapChain() { return hveRenderer_m.hveSwapChain(); }
    inline HveWindow& hveWindow() { return hveWindow_m; }
    inline GlobalUbo& globalUbo() { return ubo_; }

    inline GLFWwindow* passGLFWwindow() const { return hveWindow_m.getGLFWwindow(); } 
    
  private:
    void init();

    // construct in impl
    HveWindow hveWindow_m;

    HveDevice hveDevice_m {hveWindow_m};
    HveRenderer hveRenderer_m {hveWindow_m, hveDevice_m};

    // shared between multiple system
    u_ptr<HveDescriptorPool> globalPool_m;

    std::vector<u_ptr<HveBuffer>> uboBuffers_m {HveSwapChain::MAX_FRAMES_IN_FLIGHT};
    u_ptr<hnll::HveDescriptorSetLayout> globalSetLayout_m;
    std::vector<VkDescriptorSet> globalDescriptorSets_m {HveSwapChain::MAX_FRAMES_IN_FLIGHT};
    // ptrlize to make it later init 

    map renderingSystems_m;
    GlobalUbo ubo_{};
};

} // namespace hv