// hnll
#include <graphics/device.hpp>
#include <graphics/window.hpp>
#include <graphics/pipeline.hpp>
#include <graphics/renderer.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <gui/engine.hpp>

#include <graphics/meshlet_model.hpp>

// submodules
#include <ray_tracing_extensions.hpp>

// std
#include <memory>
#include <iostream>

namespace hnll {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;
using vec3 = Eigen::Vector3f;
using vec4 = Eigen::Vector4f;

constexpr uint32_t MAX_VERTEX_PER_MESHLET     = 64;
constexpr uint32_t MAX_INDICES_PER_MESHLET    = 378;
constexpr uint32_t MAX_MESHLET_COUNT_PER_CALL = 10;
constexpr uint32_t SCENE_SET_ID   = 2;
constexpr uint32_t VERTEX_SET_ID  = 0;
constexpr uint32_t MESHLET_SET_ID = 1;
constexpr size_t   DESC_SET_COUNT = 2;
constexpr uint32_t DEFAULT_VERTEX_COUNT  = 30000;
constexpr uint32_t DEFAULT_MESHLET_COUNT = 3000;

class mesh_pipeline : public graphics::pipeline
{
  public:
    mesh_pipeline(
      graphics::device& device,
      VkRenderPass render_pass,
      const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts) : graphics::pipeline(device)
    {
      create_pipeline_layout(descriptor_set_layouts);
      create_pipeline(render_pass);
    }
    ~mesh_pipeline()
    {
      auto device = device_.get_device();
      vkDestroyShaderModule(device, mesh_shader_module_, nullptr);
      vkDestroyShaderModule(device, frag_shader_module_, nullptr);
      vkDestroyPipelineLayout(device, layout_, nullptr);
      // vkDestroyPipeline(device_.get_device(), graphics_pipeline_, nullptr);

    }

    // getter
    VkPipelineLayout get_layout() const { return layout_; }

  private:
    void create_pipeline_layout(const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts)
    {
      VkPipelineLayoutCreateInfo create_info {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
      };
      create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
      create_info.pSetLayouts    = descriptor_set_layouts.data();
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

    void create_pipeline(VkRenderPass render_pass)
    {
      // create shader modules
      auto directory = std::string(std::getenv("HNLL_ENGN")) + "/applications/mesh_shader/introduction/shaders/spv/";
      auto mesh_shader_path = directory + "draw_triangle.mesh.glsl.spv";
      auto frag_shader_path = directory + "draw_triangle.frag.glsl.spv";
      auto mesh_shader_code = read_file(mesh_shader_path);
      auto frag_shader_code = read_file(frag_shader_path);
      create_shader_module(mesh_shader_code, &mesh_shader_module_);
      create_shader_module(frag_shader_code, &frag_shader_module_);

      // shader stages consists of mesh and frag shader (TODO : add task)
      std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
        create_shader_stage_info(VK_SHADER_STAGE_MESH_BIT_NV,  mesh_shader_module_),
        create_shader_stage_info(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader_module_),
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
      create_info.renderPass = render_pass;
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

    VkPipelineShaderStageCreateInfo create_shader_stage_info(
      VkShaderStageFlagBits stage,
      VkShaderModule        module,
      const char*           pName = "main"
      )
    {
      VkPipelineShaderStageCreateInfo shader_stage_info {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
      };
      shader_stage_info.stage  = stage;
      shader_stage_info.module = module;
      shader_stage_info.pName  = pName;
      return shader_stage_info;
    }

    //----------- variables ----------------------------------------------------
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
        VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
      };

      window_ = std::make_unique<graphics::window>(960, 840, "mesh shader introduction");
      device_ = std::make_unique<graphics::device>(
        *window_,
        graphics::rendering_type::MESH_SHADING,
        std::move(device_extensions)
      );
      // load extensions
      load_VK_EXTENSIONS(device_->get_instance(), vkGetInstanceProcAddr, device_->get_device(), vkGetDeviceProcAddr);

      renderer_ = std::make_unique<graphics::renderer>(*window_, *device_);
      gui_engine_ = std::make_unique<gui::engine>(*window_, *device_);
      renderer_->set_next_renderer(gui_engine_->renderer_p());

      plane_ = create_split_plane();

      pipeline_ = std::make_unique<mesh_pipeline>(*device_,
        renderer_->get_swap_chain_render_pass(HVE_RENDER_PASS_ID),
        plane_->get_raw_desc_set_layouts()
      );
    }

    ~mesh_shader_introduction()
    {
    }

    void run()
    {
      while (glfwWindowShouldClose(window_->get_glfw_window()) == GLFW_FALSE) {
        glfwPollEvents();
        render();
        if (!hnll::graphics::renderer::swap_chain_recreated_) {
          gui_engine_->begin_imgui();
          gui_engine_->render();
        }
      }
      vkDeviceWaitIdle(device_->get_device());
    }

  private:
    u_ptr<graphics::meshlet_model> create_split_plane()
    {
      // v3 --- v2
      //  |      |
      // v0 --- v1
      graphics::meshlet_model::vertex v0 = { vec3{-0.5f,  0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 1.f, 0.f} };
      graphics::meshlet_model::vertex v1 = { vec3{ 0.5f,  0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{1.f, 0.f, 0.f} };
      graphics::meshlet_model::vertex v2 = { vec3{ 0.5f, -0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 1.f, 0.f} };
      graphics::meshlet_model::vertex v3 = { vec3{-0.5f, -0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 0.f, 1.f} };
      std::vector<graphics::meshlet_model::vertex> raw_vertices = { v0, v1, v2, v3 };

      std::vector<graphics::meshlet<>> meshlets = {
        {{0, 1, 2}, {0, 1, 2}, 3, 3},
        {{0, 2, 3}, {0, 1, 2}, 3, 3},
      };

      return graphics::meshlet_model::create(*device_, std::move(raw_vertices), std::move(meshlets));
    }

    void render()
    {
      if (auto command_buffer = renderer_->begin_frame()) {
        int frame_index = renderer_->get_frame_index();

        renderer_->begin_swap_chain_render_pass(command_buffer, HVE_RENDER_PASS_ID);

        pipeline_->bind(command_buffer);

        // bind vertex storage buffer
        plane_->bind_and_draw(command_buffer, pipeline_->get_layout());

        renderer_->end_swap_chain_render_pass(command_buffer);
        renderer_->end_frame();
      }
    }

    u_ptr<graphics::window>   window_;
    u_ptr<graphics::device>   device_;
    u_ptr<graphics::renderer> renderer_;
    u_ptr<mesh_pipeline>      pipeline_;

    u_ptr<gui::engine> gui_engine_;

    // sample object
    u_ptr<graphics::meshlet_model> plane_;
};
} // namespace hnll

int main() {
  hnll::mesh_shader_introduction app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}