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
      std::vector<VkDescriptorSetLayout> _desc_layouts
    );

    void render(frame_info _frame_info) override;

  private:
    void create_pipeline_layout(const std::vector<VkDescriptorSetLayout>& _desc_layouts);
    void create_pipeline(
      VkRenderPass _render_pass,
      std::vector<std::string> _shader_paths,
      std::string shaders_directory = std::string(std::getenv("HNLL_ENGN")) + std::string("/modules/graphics/shader/spv/"));
};

}} // namespace hnll::graphics