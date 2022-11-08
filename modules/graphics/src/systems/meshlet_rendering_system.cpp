// hnll
#include <graphics/systems/meshlet_rendering_system.hpp>
#include <graphics/meshlet_model.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <game/components/meshlet_component.hpp>
#include <graphics/frame_info.hpp>

namespace hnll::graphics {

struct meshlet_push_constant
{
  Eigen::Matrix4f model_matrix  = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f normal_matrix = Eigen::Matrix4f::Identity();
};

meshlet_rendering_system::meshlet_rendering_system(
  hnll::graphics::device &_device,
  VkRenderPass _render_pass,
  VkDescriptorSetLayout _global_set_layout)
  : rendering_system(_device, hnll::game::render_type::MESHLET)
{
  create_pipeline_layout(_global_set_layout);
  create_pipeline(_render_pass, {"k"});
}

void meshlet_rendering_system::create_pipeline_layout(VkDescriptorSetLayout _global_set_layout)
{
  // desc sets
  auto desc_set_layouts = graphics::meshlet_model::default_desc_set_layouts(device_);
  std::vector<VkDescriptorSetLayout> raw_desc_set_layouts;

  raw_desc_set_layouts.push_back(_global_set_layout);
  // copy
  for (int i = 0; i < desc_set_layouts.size(); i++) {
    raw_desc_set_layouts.push_back(desc_set_layouts[i]->get_descriptor_set_layout());
  }

  // push constant
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags = VK_SHADER_STAGE_TASK_BIT_NV | VK_SHADER_STAGE_MESH_BIT_NV | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant_range.offset = 0;
  push_constant_range.size = sizeof(meshlet_push_constant);

  VkPipelineLayoutCreateInfo create_info {
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
  };
  create_info.setLayoutCount = static_cast<uint32_t>(raw_desc_set_layouts.size());
  create_info.pSetLayouts    = raw_desc_set_layouts.data();
  create_info.pushConstantRangeCount = 1;
  create_info.pPushConstantRanges = &push_constant_range;

  auto result = vkCreatePipelineLayout(
    device_.get_device(),
    &create_info,
    nullptr,
    &pipeline_layout_
  );
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout");
  }
}

void meshlet_rendering_system::create_pipeline(
  VkRenderPass _render_pass,
  std::vector<std::string> _shader_paths,
  std::string shaders_directory)
{
  // create shader modules
  auto directory = std::string(std::getenv("HNLL_ENGN")) + "/applications/mesh_shader/introduction/shaders/spv/";
  std::vector<std::string> shader_paths = {
    directory + "simple_meshlet.mesh.glsl.spv",
    directory + "simple_meshlet.frag.glsl.spv"
  };
  std::vector<VkShaderStageFlagBits> shader_stage_flags = {
    VK_SHADER_STAGE_MESH_BIT_NV,
    VK_SHADER_STAGE_FRAGMENT_BIT,
  };

  // configure pipeline config info
  graphics::pipeline_config_info config_info;
  graphics::pipeline::default_pipeline_config_info(config_info);
  config_info.pipeline_layout = pipeline_layout_;
  config_info.render_pass = _render_pass;

  pipeline_ = graphics::pipeline::create(
    device_,
    shader_paths,
    shader_stage_flags,
    config_info
  );
}

void meshlet_rendering_system::render(hnll::graphics::frame_info _frame_info) {
  auto command_buffer = _frame_info.command_buffer;

  pipeline_->bind(command_buffer);

  for (auto &target: render_target_map_) {

    auto obj = dynamic_cast<hnll::game::meshlet_component *>(target.second.get());

    // prepare push constant
    meshlet_push_constant push{};
    push.model_matrix = obj->get_transform().mat4().cast<float>();
    push.normal_matrix = obj->get_transform().normal_matrix().cast<float>();

    vkCmdPushConstants(
      command_buffer,
      pipeline_layout_,
      VK_SHADER_STAGE_TASK_BIT_NV | VK_SHADER_STAGE_MESH_BIT_NV | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(meshlet_push_constant),
      &push
    );
    // bind vertex storage buffer
    obj->get_model_sp()->bind(command_buffer, _frame_info.global_descriptor_set, pipeline_layout_);
    obj->get_model_sp()->draw(command_buffer);
  }
}
} // namespace hnll::graphics