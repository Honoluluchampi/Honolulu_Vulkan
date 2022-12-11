#pragma once

// hnll
#include <graphics/device.hpp>
#include <graphics/pipeline.hpp>
#include <utils/rendering_utils.hpp>

namespace hnll {

namespace game {

class shading_system
{
    using render_target_map = std::unordered_map<component_id, const renderable_component&>;

  public:
    shading_system(graphics::device& device, utils::shading_type type)
    : device_(device), shading_type_(type) {}
    virtual ~shading_system() { vkDestroyPipelineLayout(device_.get_device(), pipeline_layout_, nullptr); }

    virtual void render(const utils::frame_info& frame_info) = 0;

    void add_render_target(component_id id, const renderable_component& target)
    { render_target_map_.emplace(id, target); }
    void remove_render_target(component_id id)
    { render_target_map_.erase(id); }

    // getter
    utils::shading_type          get_shading_type()   const   { return shading_type_; }
    static VkDescriptorSetLayout get_global_desc_set_layout() { return global_desc_set_layout_; }
    static VkRenderPass          get_default_render_pass()    { return default_render_pass_; }

    // setter
    shading_system& set_shading_type(utils::shading_type type)               { shading_type_ = type; return *this; }
    static void     set_global_desc_set_layout(VkDescriptorSetLayout layout) { global_desc_set_layout_ = layout; }
    static void     set_default_render_pass(VkRenderPass pass)               { default_render_pass_ = pass; }

  protected:
    void create_pipeline(){}

    // vulkan objects
    graphics::device&         device_;
    u_ptr<graphics::pipeline> pipeline_;
    VkPipelineLayout          pipeline_layout_;

    static VkDescriptorSetLayout global_desc_set_layout_;
    static VkRenderPass          default_render_pass_;

    // shading system is called in rendering_type-order at rendering process
    utils::shading_type   shading_type_;
    render_target_map     render_target_map_;
};

namespace shading_system_helper {
// takes push_constant struct as type parameter
template <typename PushConstant>
VkPipelineLayout create_pipeline_layout(
  VkDevice device,
  VkShaderStageFlagBits shader_stage_flags,
  std::vector<VkDescriptorSetLayout> descriptor_set_layouts)
{
  // configure push constant
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags = shader_stage_flags;
  push_constant_range.offset     = 0;

  // configure desc sets layout
  VkPipelineLayoutCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
  create_info.pSetLayouts = descriptor_set_layouts.data();
  create_info.pushConstantRangeCount = 1;
  create_info.pPushConstantRanges = &push_constant_range;

  // create
  VkPipelineLayout ret;
  if (vkCreatePipelineLayout(device, &create_info, nullptr, &ret) != VK_SUCCESS)
    throw std::runtime_error("failed to create pipeline layout.");

  return ret;
}

// shaders_directory is relative to $ENV{HNLL_ENGN}
u_ptr<graphics::pipeline> create_pipeline(
  graphics::device&                  device,
  VkPipelineLayout                   pipeline_layout,
  VkRenderPass                       render_pass,
  std::string                        shaders_directory,
  std::vector<std::string>           shader_filenames,
  std::vector<VkShaderStageFlagBits> shader_stage_flags)
{
  auto directory = std::string(std::getenv("HNLL_ENGN")) + shaders_directory;

  std::vector<std::string> shader_paths;
  for (const auto& name : shader_filenames) {
    shader_paths.emplace_back(directory + name);
  }

  graphics::pipeline_config_info config_info;
  graphics::pipeline::default_pipeline_config_info(config_info);
  config_info.pipeline_layout = pipeline_layout;
  config_info.render_pass     = render_pass;

  return graphics::pipeline::create(
    device,
    shader_paths,
    shader_stage_flags,
    config_info
  );
}

} // namespace shading_system_helper
}} // namespace hnll::game