#pragma once

// hnll
#include <graphics/rendering_system.hpp>

// std
#include <vector>

namespace hnll::graphics {

class point_light_rendering_system : public rendering_system
{
  public:
    point_light_rendering_system(device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
    ~point_light_rendering_system();

    point_light_rendering_system(const point_light_rendering_system &) = delete;
    point_light_rendering_system &operator= (const point_light_rendering_system &) = delete;

    // dont make camera object as a member variable so as to share the camera between multiple render system
    void render(frame_info frame_info) override;
    
  private:
    void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
    void create_pipeline(
      VkRenderPass render_pass,
      std::string vertex_shader,
      std::string fragment_shader,
      std::string shaders_directory = std::string(std::getenv("HNLL_ENGN")) + std::string("/modules/graphics/shader/spv/"));
};

} // namespace hnll::graphics
