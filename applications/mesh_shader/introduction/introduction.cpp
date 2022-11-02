// hnll
#include <graphics/device.hpp>
#include <graphics/window.hpp>

// submodules
#include <ray_tracing_extensions.hpp>

// std
#include <memory>

namespace hnll {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

class mesh_shader_introduction {
  public:
    mesh_shader_introduction() {
      std::vector<const char *> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_NV_MESH_SHADER_EXTENSION_NAME,
        VK_NV_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME,
        VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME,
        VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME,
      };

      window_ = std::make_unique<graphics::window>(960, 840, "mesh shader introduction");
      device_ = std::make_unique<graphics::device>(
        *window_,
        graphics::rendering_type::MESH_SHADING,
        std::move(device_extensions)
      );
      // load extensions
      load_VK_EXTENSIONS(device_->get_instance(), vkGetInstanceProcAddr, device_->get_device(), vkGetDeviceProcAddr);
    }

    void run()
    {
      init();
      while (glfwWindowShouldClose(window_->get_glfw_window()) == GLFW_FALSE) {
        glfwPollEvents();
        render();
      }
      vkDeviceWaitIdle(device_->get_device());
    }

    void init()
    {
      create_layout();
      create_pipeline();
    }

  private:
    void create_layout()
    {

    }

    void create_pipeline()
    {

    }

    void render()
    {
      VkCommandBuffer command;
      vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipeline_);
      uint32_t num_work_groups = 1;
      vkCmdDrawMeshTasksNV(command, num_work_groups, 0);
      vkCmdDrawMeshTasks
    }

    u_ptr<graphics::window> window_;
    u_ptr<graphics::device> device_;

    VkPipeline mesh_pipeline_;
};
} // namespace hnll

int main() {
  hnll::mesh_shader_introduction app{};
}