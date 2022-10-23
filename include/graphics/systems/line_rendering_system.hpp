#pragma once

// hnll
#include <graphics/rendering_system.hpp>

// lib
#include <vulkan/vulkan.hpp>

// std
#include <vector>

namespace hnll::graphics {

class line_rendering_system : public rendering_system
{
  public:
    line_rendering_system(device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
    ~line_rendering_system() override;

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
