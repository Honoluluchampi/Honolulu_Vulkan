#pragma once

// hnll
#include <graphics/rendering_system.hpp>

namespace hnll {

namespace graphics {

// forward declaration
class descriptor_pool;
class descriptor_set_layout;
class buffer;

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
    void setup_task_desc();
    void create_pipeline_layout(VkDescriptorSetLayout _global_layouts);
    void create_pipeline_layout_without_task(VkDescriptorSetLayout _global_layouts);
    void create_pipeline(
      VkRenderPass _render_pass,
      std::vector<std::string> _shader_paths,
      std::string shaders_directory = std::string(std::getenv("HNLL_ENGN")) + std::string("/modules/graphics/shader/spv/"));
    void create_pipeline_without_task(
      VkRenderPass _render_pass,
      std::vector<std::string> _shader_paths,
      std::string shaders_directory = std::string(std::getenv("HNLL_ENGN")) + std::string("/modules/graphics/shader/spv/"));

    u_ptr<descriptor_pool>       task_desc_pool_;
    std::vector<u_ptr<buffer>>   task_desc_buffers_;
    u_ptr<descriptor_set_layout> task_desc_layout_;
    std::vector<VkDescriptorSet> task_desc_sets_;

    // switching graphics pipeline
    VkPipelineLayout pipeline_layout_without_task_;
    VkPipeline pipeline_without_task_;
    VkPipeline current_pipeline_;
};

}} // namespace hnll::graphics