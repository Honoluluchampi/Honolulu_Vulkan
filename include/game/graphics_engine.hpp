#pragma once

// hnll
#include <graphics/window.hpp>
#include <graphics/device.hpp>
#include <graphics/renderer.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <graphics/buffer.hpp>
#include <graphics/rendering_system.hpp>
#include <utils/rendering_utils.hpp>

// std
#include <map>

namespace hnll::game {

// forward declaration
class shading_system;

class graphics_engine
{
    using shading_system_map = std::map<uint32_t, u_ptr<shading_system>>;
  public:
    static constexpr int WIDTH = 960;
    static constexpr int HEIGHT = 820;
    static constexpr float MAX_FRAME_TIME = 0.05;

    graphics_engine(const char* window_name = "honolulu engine", utils::rendering_type rendering_type = utils::rendering_type::MESH_SHADING);
    ~graphics_engine();

    graphics_engine(const graphics_engine &) = delete;
    graphics_engine &operator= (const graphics_engine &) = delete;

    void render(const utils::viewer_info& _viewer_info, const utils::frustum_info& _frustum_info);

    void configure_shading_system();
    static void add_shading_system(u_ptr<shading_system>&& shading_system);
    static void add_renderable_component(const renderable_component &comp);
    static void remove_renderable_component(const renderable_component &comp);
    static void remove_renderable_component(utils::shading_type type, component_id id);

    inline void wait_idle() { vkDeviceWaitIdle(device_->get_device()); }
    void update_ubo(int frame_index)
    { ubo_buffers_[frame_index]->write_to_buffer(&ubo_); ubo_buffers_[frame_index]->flush(); }

    inline graphics::device&     get_device_r()     { return *device_; }
    inline graphics::renderer&   get_renderer_r()   { return *renderer_; }
    inline graphics::swap_chain& get_swap_chain_r() { return renderer_->get_swap_chain(); }
    inline graphics::window&     get_window_r()     { return *window_; }
    inline utils::global_ubo&    get_global_ubo_r() { return ubo_; }

    inline VkDescriptorSetLayout get_global_desc_set_layout() const { return global_set_layout_->get_descriptor_set_layout(); }
    inline VkDescriptorSet       get_global_descriptor_set(int frame_index) { return global_descriptor_sets_[frame_index]; }

    inline GLFWwindow* get_glfw_window() const { return window_->get_glfw_window(); }
    
  private:
    void init();

    // construct in impl
    u_ptr<graphics::window>   window_;
    u_ptr<graphics::device>   device_;
    u_ptr<graphics::renderer> renderer_;

    static shading_system_map shading_systems_;

    // shared between multiple system
    u_ptr<graphics::descriptor_pool>       global_pool_;
    utils::global_ubo                      ubo_{};
    std::vector<u_ptr<graphics::buffer>>   ubo_buffers_ {graphics::swap_chain::MAX_FRAMES_IN_FLIGHT};
    u_ptr<graphics::descriptor_set_layout> global_set_layout_;
    std::vector<VkDescriptorSet>           global_descriptor_sets_ {graphics::swap_chain::MAX_FRAMES_IN_FLIGHT};
};

} // namespace hnll::graphics
