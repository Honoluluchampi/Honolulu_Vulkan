// hnll
#include <graphics/device.hpp>
#include <graphics/window.hpp>
#include <graphics/pipeline.hpp>
#include <graphics/renderer.hpp>
#include <gui/engine.hpp>
#include <graphics/utils.hpp>
#include <graphics/meshlet_model.hpp>

// submodules
#include <ray_tracing_extensions.hpp>

// std
#include <memory>
#include <iostream>

namespace hnll {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;
using vec2 = Eigen::Vector2f;
using vec3 = Eigen::Vector3f;
using vec4 = Eigen::Vector4f;

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
      create_pipeline();
    }

    ~mesh_shader_introduction()
    {
      vkDestroyPipelineLayout(device_->get_device(), pipeline_layout_, nullptr);
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
    // following two functions will be migrated into mesh_rendering_system
    void create_pipeline()
    {
      // create shader modules
      auto directory = std::string(std::getenv("HNLL_ENGN")) + "/applications/mesh_shader/introduction/shaders/spv/";
      std::vector<std::string> shader_paths = {
        directory + "simple_meshlet.mesh.glsl.spv",
        directory + "simple_meshlet.frag.glsl.spv"
      };
      std::vector<VkShaderStageFlagBits> shader_stage_flags = {
        VK_SHADER_STAGE_MESH_BIT_NV,
        VK_SHADER_STAGE_FRAGMENT_BIT,
      };

      // init pipeline_layout
      create_pipeline_layout();
      // configure pipeline config info
      graphics::pipeline_config_info config_info;
      graphics::pipeline::default_pipeline_config_info(config_info);
      config_info.pipeline_layout = pipeline_layout_;
      config_info.render_pass = renderer_->get_swap_chain_render_pass(HVE_RENDER_PASS_ID);

      pipeline_ = graphics::pipeline::create(
        *device_,
        shader_paths,
        shader_stage_flags,
        config_info
      );
    }

    void create_pipeline_layout()
    {
      auto desc_set_layouts = graphics::meshlet_model::default_desc_set_layouts(*device_);
      std::vector<VkDescriptorSetLayout> raw_desc_set_layouts(desc_set_layouts.size());

      // copy
      for (int i = 0; i < desc_set_layouts.size(); i++) {
        raw_desc_set_layouts[i] = desc_set_layouts[i]->get_descriptor_set_layout();
      }

      VkPipelineLayoutCreateInfo create_info {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
      };
      create_info.setLayoutCount = static_cast<uint32_t>(desc_set_layouts.size());
      create_info.pSetLayouts    = raw_desc_set_layouts.data();
      create_info.pushConstantRangeCount = 0;
      auto result = vkCreatePipelineLayout(
        device_->get_device(),
        &create_info,
        nullptr,
        &pipeline_layout_
      );
      if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
      }
    }

    u_ptr<graphics::meshlet_model> create_split_plane()
    {
      // v3 --- v2
      //  |  /  |
      // v0 --- v1
      graphics::vertex v0 = { vec3{-0.5f,  0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 1.f, 0.f}, vec2{0.f, 0.f} };
      graphics::vertex v1 = { vec3{ 0.5f,  0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{1.f, 0.f, 0.f}, vec2{0.f, 0.f} };
      graphics::vertex v2 = { vec3{ 0.5f, -0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 1.f, 0.f}, vec2{0.f, 0.f} };
      graphics::vertex v3 = { vec3{-0.5f, -0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 0.f, 1.f}, vec2{0.f, 0.f} };
      std::vector<graphics::vertex> raw_vertices = { v0, v1, v2, v3 };

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
        plane_->bind_and_draw(command_buffer, pipeline_layout_);

        renderer_->end_swap_chain_render_pass(command_buffer);
        renderer_->end_frame();
      }
    }

    u_ptr<graphics::window>   window_;
    u_ptr<graphics::device>   device_;
    u_ptr<graphics::renderer> renderer_;
    u_ptr<graphics::pipeline> pipeline_;
    VkPipelineLayout          pipeline_layout_;

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