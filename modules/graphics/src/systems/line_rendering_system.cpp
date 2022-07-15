// hnll
#include <graphics/systems/line_rendering_system.hpp>

// hge
#include <game/components/line_component.hpp>

namespace hnll {
namespace graphics {

int line_rendering_system::inter_polating_points_count = 4;

// should be compatible with a shader
// TODO use head and tail
struct line_push_constant
{
  glm::vec4 position{};
  glm::vec4 color{1.f};
  float radius;
};

line_rendering_system::line_rendering_system
  (device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout)
  : rendering_system(device, hnll::game::render_type::LINE)
{
  create_pipeline_layout(global_set_layout);
  create_pipeline(render_pass);
}

line_rendering_system::~line_rendering_system()
{}

// TODO : take pushConstanctRange.size and virtualize this function
void line_rendering_system::create_pipeline_layout(VkDescriptorSetLayout global_set_layout)
{
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant_range.offset = 0;
  push_constant_range.size = sizeof(line_push_constant);

  std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};
  
  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
  pipeline_layout_info.pushConstantRangeCount = 1;
  pipeline_layout_info.pPushConstantRanges = &push_constant_range;
  if (vkCreatePipelineLayout(device_.get_device(), &pipeline_layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
    throw std::runtime_error("failed to create pipeline layout!");
}

void line_rendering_system::create_pipeline(VkRenderPass render_pass)
{
  assert(pipeline_layout_ != nullptr && "cannot create pipeline before pipeline layout!");

  pipeline_config_info pipeline_config{};
  pipeline::default_pipeline_config_info(pipeline_config);
  // for original configuration (dont use any vertex input attribute)
  pipeline_config.attribute_descriptions.clear();
  pipeline_config.binding_descriptions.clear();

  pipeline_config.render_pass = render_pass;
  pipeline_config.pipeline_layout = pipeline_layout_;
  pipeline_ = std::make_unique<pipeline>(
    device_,
    std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/point_light.vert.spv"),
    std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/point_light.frag.spv"),
    pipeline_config
  );
}

void line_rendering_system::render(frame_info frame_info)
{
  pipeline_->bind(frame_info.command_buffer);

  vkCmdBindDescriptorSets(
    frame_info.command_buffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline_layout_,
    0, 1,
    &frame_info.global_discriptor_set,
    0, nullptr
  );

  for (auto& target : render_target_map_) {
    
    auto obj = dynamic_cast<hnll::game::line_component*>(target.second.get());

    auto head_to_tail = obj->get_tail() - obj->get_head();
    // TODO : draw line
    for (int i = 1; i < inter_polating_points_count + 1; i++) {
      // fill push constant
      line_push_constant push{};
      push.position = glm::vec4(head_to_tail, 0.f);
      push.position *= (float)i / (inter_polating_points_count + 1);
      push.position += glm::vec4(obj->get_head(), 0.f);
      push.color = glm::vec4(obj->get_color(), 0.f);
      push.radius = obj->get_radius();

      vkCmdPushConstants(
          frame_info.command_buffer,
          pipeline_layout_, 
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
          0, 
          sizeof(line_push_constant), 
          &push
      );
      vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
    }
  }
}

} // namespace graphics
} // namespace hnll