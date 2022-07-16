#pragma once

// hnll
#include <graphics/rendering_system.hpp>

// lib
#include <vulkan/vulkan.hpp>

// std
#include <vector>

namespace hnll{
namespace graphics {

class line_rendering_system : public rendering_system
{
  public:
    line_rendering_system(device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
    ~line_rendering_system();

    void render(frame_info frameInfo) override;
    
    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
    
    // TODO : delete
    static int inter_polating_points_count;
  private:
    void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) override;
    void create_pipeline(
      VkRenderPass render_pass,
      std::string vertex_shader,
      std::string fragment_shader,
      std::string shaders_directory = std::string(std::getenv("HNLL_ENGN")) + std::string("/modules/graphics/shader/spv/")) override;
};

} // namespace graphics
} // namespace hnll