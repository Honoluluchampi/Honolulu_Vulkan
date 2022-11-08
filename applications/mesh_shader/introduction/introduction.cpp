// hnll
#include <graphics/engine.hpp>
#include <gui/engine.hpp>
#include <graphics/utils.hpp>
#include <graphics/meshlet_model.hpp>

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
      graphics_engine_ = std::make_unique<graphics::engine>(
        "mesh shader introduction",
        graphics::rendering_type::MESH_SHADING
      );

      gui_engine_ = std::make_unique<gui::engine>(graphics_engine_->get_window(), graphics_engine_->get_device());
      graphics_engine_->get_renderer().set_next_renderer(gui_engine_->renderer_p());

      plane_ = create_split_plane();
    }

    ~mesh_shader_introduction()
    {
    }

    void run()
    {
      while (glfwWindowShouldClose(graphics_engine_->get_glfw_window()) == GLFW_FALSE) {
        glfwPollEvents();
        render();
        if (!hnll::graphics::renderer::swap_chain_recreated_) {
          gui_engine_->begin_imgui();
          gui_engine_->render();
        }
      }
      graphics_engine_->wait_idle();
    }

  private:
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

      std::vector<graphics::meshlet> meshlets = {
        {{0, 1, 2}, {0, 1, 2}, 3, 3},
        {{0, 2, 3}, {0, 1, 2}, 3, 3},
      };

      return graphics::meshlet_model::create(graphics_engine_->get_device(), std::move(raw_vertices), std::move(meshlets));
    }

    void render()
    {
      utils::viewer_info vi{};
      graphics_engine_->render(std::move(vi));
    }

    VkPipelineLayout          pipeline_layout_;
    u_ptr<graphics::engine>   graphics_engine_;
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