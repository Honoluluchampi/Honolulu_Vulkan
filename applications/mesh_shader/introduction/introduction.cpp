// hnll
#include <graphics/device.hpp>
#include <graphics/window.hpp>
#include <graphics/pipeline.hpp>
#include <graphics/renderer.hpp>

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
      create_layout();
      create_pipeline();
    }
    ~mesh_pipeline()
    {
      // vkDestroyShaderModule
      // vkDestroyPipeline(device_.get_device(), graphics_pipeline_, nullptr);
    }

    // getter
    VkPipelineLayout get_layout() const { return layout_; }

  private:
    void create_layout()
    {
      VkPipelineLayoutCreateInfo create_info {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
      };
      create_info.setLayoutCount = 0;
      create_info.pushConstantRangeCount = 0;
      auto result = vkCreatePipelineLayout(
        device_.get_device(),
        &create_info,
        nullptr,
        &layout_
      );
      if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
      }
    }

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
      std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
        create_mesh_shader_stage_info(),
        create_frag_shader_stage_info(),
      };

      graphics::pipeline_config_info config_info;
      default_pipeline_config_info(config_info);

      VkGraphicsPipelineCreateInfo create_info {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
      };
      create_info.stageCount = shader_stages.size();
      create_info.pStages = shader_stages.data();

      // use config_info if necessary
      create_info.pInputAssemblyState = &config_info.input_assembly_info;
      create_info.pViewportState      = &config_info.viewport_info;
      create_info.pRasterizationState = &config_info.rasterization_info;
      create_info.pMultisampleState   = &config_info.multi_sample_info;
      create_info.pColorBlendState    = &config_info.color_blend_info;
      create_info.pDepthStencilState  = &config_info.depth_stencil_info;
      create_info.pDynamicState       = &config_info.dynamic_state_info;

      create_info.layout     = layout_;
      create_info.renderPass = config_info.render_pass;
      create_info.subpass    = config_info.subpass;

      create_info.basePipelineIndex = -1;
      create_info.basePipelineHandle = VK_NULL_HANDLE;

      auto result = vkCreateGraphicsPipelines(
        device_.get_device(),
        VK_NULL_HANDLE,
        1,
        &create_info,
        nullptr,
        &graphics_pipeline_
      );
      if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline");
      }
    }

    VkPipelineShaderStageCreateInfo create_mesh_shader_stage_info()
    {
      VkPipelineShaderStageCreateInfo mesh_shader_stage_info {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
      };
      mesh_shader_stage_info.stage = VK_SHADER_STAGE_MESH_BIT_NV;
      mesh_shader_stage_info.module = mesh_shader_module_;
      mesh_shader_stage_info.pName = "main"; // int main() of mesh shader
      return mesh_shader_stage_info;
    }

    VkPipelineShaderStageCreateInfo create_frag_shader_stage_info()
    {
      VkPipelineShaderStageCreateInfo frag_shader_stage_info {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
      };
      frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      frag_shader_stage_info.module = frag_shader_module_;
      frag_shader_stage_info.pName = "main";
      return frag_shader_stage_info;
    }

    VkShaderModule mesh_shader_module_;
    VkShaderModule frag_shader_module_;

    VkPipelineLayout layout_;
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

      pipeline_ = std::make_unique<mesh_pipeline>(*device_);
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
    }

  private:
    void create_layout()
    {

    }

    void render()
    {
      VkCommandBuffer command;
//      pipeline_->bind(command);
      uint32_t num_work_groups = 1;
//      vkCmdDrawMeshTasksNV(command, num_work_groups, 0);
    }

    u_ptr<graphics::window>   window_;
    u_ptr<graphics::device>   device_;
    u_ptr<graphics::renderer> renderer_;
    u_ptr<mesh_pipeline>      pipeline_;
};
} // namespace hnll

int main() {
  hnll::mesh_shader_introduction app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}