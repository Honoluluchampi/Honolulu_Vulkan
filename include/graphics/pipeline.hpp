#pragma once

// hnll
#include <graphics/device.hpp>

// std
#include <vector>
#include <string>

namespace hnll::graphics {

struct pipeline_config_info 
{
  pipeline_config_info() = default;
  pipeline_config_info(const pipeline_config_info&) = delete;
  pipeline_config_info& operator=(const pipeline_config_info&) = delete;
  // for pipeline config info
  void create_input_assembly_info();
  // viewport and scissor
  void create_viewport_info();
  // rasterizer
  void create_rasterization_info();
  // multisampling used for anti-aliasing
  void create_multi_sample_state();
  // color blending for alpha blending
  void create_color_blend_attachment();
  void create_color_blend_state();
  void create_depth_stencil_state();
  // dynamic state
  void create_dynamic_state();

  // member variables
  std::vector<VkVertexInputBindingDescription> binding_descriptions{};
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
  VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
  VkPipelineViewportStateCreateInfo viewport_info{};
  VkPipelineRasterizationStateCreateInfo rasterization_info{};
  VkPipelineMultisampleStateCreateInfo multi_sample_info{};
  VkPipelineColorBlendAttachmentState color_blend_attachment{};
  VkPipelineColorBlendStateCreateInfo color_blend_info{};
  VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};
  std::vector<VkDynamicState> dynamic_state_enables;
  VkPipelineDynamicStateCreateInfo dynamic_state_info{};
  VkPipelineLayout pipeline_layout = nullptr;
  VkRenderPass render_pass = nullptr;
  uint32_t subpass = 0;
};

class pipeline
{
  public:
    pipeline(
        device &device,
        const std::string &vert_filepath,
        const std::string &frag_filepath,
        const pipeline_config_info &config_info);
    ~pipeline();

    // uncopyable
    pipeline(const pipeline &) = delete;
    pipeline& operator=(const pipeline &) = delete;

    void bind(VkCommandBuffer command_buffer);

    static void default_pipeline_config_info(pipeline_config_info &config_info);
    static void enable_alpha_blending(pipeline_config_info& config_info);

  private:
    // fstream can only output char not std::string
    static std::vector<char> read_file(const std::string &filepath);

    void create_graphics_pipeline(
      const std::string &vert_filepath, 
      const std::string &frag_filepath, 
      const pipeline_config_info &config_info);

    void create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module);

    VkPipelineShaderStageCreateInfo create_vertex_shader_stage_info();
    VkPipelineShaderStageCreateInfo create_fragment_shader_stage_info();
    VkPipelineVertexInputStateCreateInfo create_vertex_input_info();
    // 
    device& device_;
    VkPipeline graphics_pipeline_;
    VkShaderModule vertex_shader_module_;
    VkShaderModule fragment_shader_module_;
};

} // namespace hnll::graphics