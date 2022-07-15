#pragma once

// hnll
#include <graphics/window.hpp>
#include <graphics/device.hpp>
#include <graphics/renderer.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <graphics/camera.hpp>
#include <graphics/buffer.hpp>
#include <graphics/rendering_system.hpp>
#include <game/components/viewer_component.hpp>

// std
#include <memory>
#include <vector>
#include <chrono>
#include <stdexcept>
#include <unordered_map>

namespace hnll {
namespace graphics {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

class engine
{
  using map = std::unordered_map<hnll::game::render_type, std::unique_ptr<HveRenderingSystem>>;

  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;
    static constexpr float MAX_FRAME_TIME = 0.05;

    engine(const char* windowName = "honolulu engine");
    ~engine();

    engine(const engine &) = delete;
    engine &operator= (const engine &) = delete;

    void render(viewer_component& viewerComp);

    // takes s_ptr<RenderableComponent>
    template<class RC>
    void set_renderable_component(RC&& target)
    { renderingSystems_m[target->get_render_type()]->addRenderTarget(target->get_id(), std::forward<RC>(target)); }
    
    template<class RC>
    void replace_renderable_component(RC&& target)
    { renderingSystems_m[target->get_render_type()]->replaceRenderTarget(target->get_id(), std::forward<RC>(target)); }

    void remove_renderable_component_without_owner(hnll::game::render_type type, hnll::game::component::id id);

    inline void waitIdle() { vkDeviceWaitIdle(hveDevice_m.device()); }

    inline device& get_device() { return hveDevice_m; }
    inline renderer& hveRenderer() { return renderer_; }
    inline HveSwapChain& hveSwapChain() { return renderer_.hveSwapChain(); }
    inline HveWindow& hveWindow() { return hveWindow_m; }
    inline global_ubo& globalUbo() { return ubo_; }

    inline GLFWwindow* passGLFWwindow() const { return hveWindow_m.getGLFWwindow(); } 
    
  private:
    void init();

    // construct in impl
    HveWindow hveWindow_m;

    device hveDevice_m {hveWindow_m};
    renderer renderer_ {hveWindow_m, hveDevice_m};

    // shared between multiple system
    u_ptr<HveDescriptorPool> globalPool_m;

    std::vector<u_ptr<HveBuffer>> uboBuffers_m {HveSwapChain::MAX_FRAMES_IN_FLIGHT};
    u_ptr<hnll::HveDescriptorSetLayout> globalSetLayout_m;
    std::vector<VkDescriptorSet> globalDescriptorSets_m {HveSwapChain::MAX_FRAMES_IN_FLIGHT};
    // ptrlize to make it later init 

    map renderingSystems_m;
    global_ubo ubo_{};
};

} // namespace graphics
} // namespace hnll