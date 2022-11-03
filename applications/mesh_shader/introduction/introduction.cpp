// hnll
#include <graphics/device.hpp>
#include <graphics/window.hpp>
#include <graphics/pipeline.hpp>
#include <graphics/renderer.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <gui/engine.hpp>

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

constexpr uint32_t MAX_VERTEX_PER_MESHLET = 64;
constexpr uint32_t MAX_PRIMITIVE_PER_MESHLET = 126;
constexpr uint32_t MAX_MESHLET_COUNT_PER_CALL = 10;

class mesh
{
  public:
    struct vertex
    {
      vec3 position;
      vec3 normal;
      vec3 color;
    };

    // pass to the mesh shader via descriptor set
    struct meshlet
    {
      uint32_t vertex_indices[MAX_VERTEX_PER_MESHLET]; // indicates position in a vertex buffer
      uint32_t primitive_indices[MAX_PRIMITIVE_PER_MESHLET];
      uint32_t vertex_count;
      uint32_t index_count;
    };

    mesh(graphics::device& device, std::vector<vertex>&& vertices, std::vector<meshlet>&& meshlets)
      : device_(device), raw_vertices_(std::move(vertices)), meshlets_(std::move(meshlets))
    {
      // buffer creation
      vertex_buffer_ = create_buffer_with_staging(
        sizeof(vertex),
        raw_vertices_.size(),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      );
      meshlet_buffer_ = create_buffer_with_staging(
        sizeof(meshlet),
        meshlets_.size(),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      );
    }
    ~mesh(){}

  private:
    u_ptr<graphics::buffer> create_buffer_with_staging(
      uint32_t instance_size,
      uint32_t instance_count,
      VkBufferUsageFlagBits usage,
      VkMemoryPropertyFlagBits memory_props
      )
    {
      graphics::buffer staging_buffer (
        device_,
        instance_size,
        instance_count,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      );

      staging_buffer.map();
      staging_buffer.write_to_buffer((void *) raw_vertices_.data());

      auto ret = std::make_unique<graphics::buffer>(
        device_,
        instance_size,
        instance_count,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
        memory_props
      );

      VkDeviceSize buffer_size = instance_size * instance_count;
      device_.copy_buffer(staging_buffer.get_buffer(), ret->get_buffer(), buffer_size);

      return ret;
    }

    graphics::device& device_;
    u_ptr<graphics::buffer> vertex_buffer_;
    u_ptr<graphics::buffer> meshlet_buffer_;
    std::vector<vertex>  raw_vertices_;
    std::vector<meshlet> meshlets_;
    uint32_t vertex_count_;
    uint32_t meshlet_count_;
};

class mesh_pipeline : public graphics::pipeline
{
  public:
    mesh_pipeline(graphics::device& device, VkRenderPass render_pass) : graphics::pipeline(device)
    {
      create_layout();
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

      pipeline_ = std::make_unique<mesh_pipeline>(*device_,
        renderer_->get_swap_chain_render_pass(HVE_RENDER_PASS_ID));

      create_descriptor();
    }

    ~mesh_shader_introduction()
    {
    }

    void run()
    {
      init();
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

    void init()
    {
      create_layout();
    }

  private:
    void create_layout()
    {

    }

    void create_descriptor()
    {
      desc_pool_ = graphics::descriptor_pool::builder(*device_)
        .set_max_sets(graphics::swap_chain::MAX_FRAMES_IN_FLIGHT)
        .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, graphics::swap_chain::MAX_FRAMES_IN_FLIGHT)
        .build();
//
//      for (int i = 0; i < desc_buffers_.size(); i++) {
//        desc_buffers_[i] = std::make_unique<graphics::buffer> (
//          *device_,
//          sizeof(mesh),
//          );
//      }
    }

    u_ptr<mesh> create_split_plane()
    {
      // v3 --- v2
      //  |      |
      // v0 --- v1
      mesh::vertex v0 = { vec3{-0.5f,  0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 1.f, 0.f} };
      mesh::vertex v1 = { vec3{ 0.5f,  0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{1.f, 0.f, 0.f} };
      mesh::vertex v2 = { vec3{ 0.5f, -0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 1.f, 0.f} };
      mesh::vertex v3 = { vec3{-0.5f, -0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 0.f, 1.f} };
      std::vector<mesh::vertex> raw_vertices = { v0, v1, v2, v3 };

      std::vector<mesh::meshlet> meshlets = {
        {{0, 1, 2}, {0, 1, 2}, 3, 3},
        {{0, 2, 3}, {0, 1, 2}, 3, 3},
      };

      return std::make_unique<mesh>(*device_, std::move(raw_vertices), std::move(meshlets));
    }

    void render()
    {
      if (auto command_buffer = renderer_->begin_frame()) {
        int frame_index = renderer_->get_frame_index();

        renderer_->begin_swap_chain_render_pass(command_buffer, HVE_RENDER_PASS_ID);

        pipeline_->bind(command_buffer);
        uint32_t num_work_groups = 1;
        vkCmdDrawMeshTasksNV(command_buffer, num_work_groups, 0);

        renderer_->end_swap_chain_render_pass(command_buffer);
        renderer_->end_frame();
      }
    }

    u_ptr<graphics::window>   window_;
    u_ptr<graphics::device>   device_;
    u_ptr<graphics::renderer> renderer_;
    u_ptr<mesh_pipeline>      pipeline_;

    u_ptr<gui::engine> gui_engine_;

    // descriptor
    u_ptr<graphics::descriptor_pool>       desc_pool_;
    u_ptr<graphics::descriptor_set_layout> desc_layout_;
    std::vector<VkDescriptorSet>           desc_sets_;
    std::vector<u_ptr<graphics::buffer>>   desc_buffers_;
};
} // namespace hnll

int main() {
  hnll::mesh_shader_introduction app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}