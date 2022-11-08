#pragma once

// hnll
#include <graphics/rendering_system.hpp>

namespace hnll {

namespace graphics {

class meshlet_rendering_system : public rendering_system
{
  public:
    meshlet_rendering_system(
      device& _device,
      VkRenderPass _render_pass,
      VkDescriptorSetLayout _global_set_layouts
    );

    void render(frame_info _frame_info) override;

  private:
    std::vector<VkDescriptorSetLayout> create_desc_layout(VkDescriptorSetLayout _global_set_layout);
    void create_pipeline_layout(VkDescriptorSetLayout _global_layouts);
    void create_pipeline(
      VkRenderPass _render_pass,
      std::vector<std::string> _shader_paths,
      std::string shaders_directory = std::string(std::getenv("HNLL_ENGN")) + std::string("/modules/graphics/shader/spv/"));
};

}} // namespace hnll::graphics