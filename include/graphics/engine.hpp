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
  using map = std::unordered_map<hnll::game::render_type, std::unique_ptr<rendering_system>>;

  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;
    static constexpr float MAX_FRAME_TIME = 0.05;

    engine(const char* window_name = "honolulu engine");
    ~engine();

    engine(const engine &) = delete;
    engine &operator= (const engine &) = delete;

    void render(hnll::game::viewer_component& viewer_comp);

    // takes s_ptr<RenderableComponent>
    template<class RC>
    void set_renderable_component(RC&& target)
    { rendering_systems_[target->get_render_type()]->add_render_target(target->get_id(), std::forward<RC>(target)); }
    
    template<class RC>
    void replace_renderable_component(RC&& target)
    { rendering_systems_[target->get_render_type()]->replace_render_target(target->get_id(), std::forward<RC>(target)); }

    void remove_renderable_component_without_owner(hnll::game::render_type type, hnll::game::component::id id);

    inline void wait_idle() { vkDeviceWaitIdle(device_.get_device()); }

    inline device& get_device() { return device_; }
    inline renderer& get_renderer() { return renderer_; }
    inline swap_chain& get_swap_chain() { return renderer_.get_swap_chain(); }
    inline window& get_window() { return window_; }
    inline global_ubo& get_global_ubo() { return ubo_; }

    inline GLFWwindow* get_glfw_window() const { return window_.get_glfw_window(); } 
    
  private:
    void init();

    // construct in impl
    window window_;

    device device_ {window_};
    renderer renderer_ {window_, device_};

    // shared between multiple system
    u_ptr<descriptor_pool> global_pool_;

    std::vector<u_ptr<buffer>> ubo_buffers_ {swap_chain::MAX_FRAMES_IN_FLIGHT};
    u_ptr<descriptor_set_layout> global_set_layout_;
    std::vector<VkDescriptorSet> global_descriptor_sets_ {swap_chain::MAX_FRAMES_IN_FLIGHT};
    // ptrlize to make it later init 

    map rendering_systems_;
    global_ubo ubo_{};
};

} // namespace graphics
} // namespace hnll