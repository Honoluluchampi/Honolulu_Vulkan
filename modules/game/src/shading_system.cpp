// hnll
#include <game/shading_system.hpp>

namespace hnll::game {

template<>
VkPipelineLayout shading_system::create_pipeline_layout<no_push_constant>(
  VkShaderStageFlagBits shader_stage_flags,
  std::vector<VkDescriptorSetLayout> descriptor_set_layouts)
{
  // configure desc sets layout
  VkPipelineLayoutCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
  create_info.pSetLayouts = descriptor_set_layouts.data();
  create_info.pushConstantRangeCount = 0;
  create_info.pPushConstantRanges = nullptr;

  // create
  VkPipelineLayout ret;
  if (vkCreatePipelineLayout(device_.get_device(), &create_info, nullptr, &ret) != VK_SUCCESS)
    throw std::runtime_error("failed to create pipeline layout.");

  return ret;
}

// shaders_directory is relative to $ENV{HNLL_ENGN}
u_ptr<graphics::pipeline> shading_system::create_pipeline(
  VkPipelineLayout                   pipeline_layout,
  VkRenderPass                       render_pass,
  std::string                        shaders_directory,
  std::vector<std::string>           shader_filenames,
  std::vector<VkShaderStageFlagBits> shader_stage_flags,
  graphics::pipeline_config_info     config_info)
{
  auto directory = std::string(std::getenv("HNLL_ENGN")) + shaders_directory;

  std::vector<std::string> shader_paths;
  for (const auto& name : shader_filenames) {
    shader_paths.emplace_back(directory + name);
  }

  config_info.pipeline_layout = pipeline_layout;
  config_info.render_pass     = render_pass;

  return graphics::pipeline::create(
    device_,
    shader_paths,
    shader_stage_flags,
    config_info
  );
}

} // namespace hnll::game