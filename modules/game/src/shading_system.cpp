// hnll
#include <game/shading_system.hpp>

namespace hnll::game {

// shaders_directory is relative to $ENV{HNLL_ENGN}
u_ptr<graphics::pipeline> shading_system::create_pipeline(
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
    device_,
    shader_paths,
    shader_stage_flags,
    config_info
  );
}

} // namespace hnll::game