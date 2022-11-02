// hnll
#include <graphics/device.hpp>
#include <graphics/window.hpp>
#include <graphics/pipeline.hpp>

// submodules
#include <ray_tracing_extensions.hpp>

// std
#include <memory>
#include <iostream>

namespace hnll {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

class mesh_pipeline : public graphics::pipeline
{
  public:
    mesh_pipeline(graphics::device& device) : graphics::pipeline(device)
    {
      create_pipeline();
    }
    ~mesh_pipeline()
    {
      // vkDestroyShaderModule
      // vkDestroyPipeline(device_.get_device(), graphics_pipeline_, nullptr);
    }
  private:
    void create_pipeline()
    {
      // create shader modules
      auto directory = std::string(std::getenv("HNLL_ENGN")) + "/applications/mesh_shader/introduction/shaders/spv/";
      auto mesh_shader_path = directory + "draw_triangle.mesh.spv";
      auto frag_shader_path = directory + "draw_triangle.frag.spv";
      auto mesh_shader_code = read_file(mesh_shader_path);
      auto frag_shader_code = read_file(frag_shader_path);
      create_shader_module(mesh_shader_code, &mesh_shader_module_);
      create_shader_module(frag_shader_code, &frag_shader_module_);

      // shader stages consists of mesh and frag shader (TODO : add task)
      VkPipelineShaderStageCreateInfo shader_stages[2] = {

      };
    }

    VkShaderModule mesh_shader_module_;
    VkShaderModule frag_shader_module_;
};

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
//      pipeline_->bind(command);
      uint32_t num_work_groups = 1;
//      vkCmdDrawMeshTasksNV(command, num_work_groups, 0);
    }

    u_ptr<graphics::window> window_;
    u_ptr<graphics::device> device_;

    u_ptr<mesh_pipeline> pipeline_;
    VkPipelineLayout pipeline_layout_;
};
} // namespace hnll

int main() {
  hnll::mesh_shader_introduction app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}