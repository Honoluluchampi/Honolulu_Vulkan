// hnll
#include <game/shading_systems/wire_frustum_shading_system.hpp>
#include <game/components/wire_frame_frustum_component.hpp>
#include <utils/common_using.hpp>

namespace hnll::game {

struct wire_frustum_push_constant
{
  mat4 model_mat;
};

u_ptr<wire_frustum_shading_system> wire_frustum_shading_system::create(graphics::device &device)
{ return std::make_unique<wire_frustum_shading_system>(device); }

wire_frustum_shading_system::wire_frustum_shading_system(graphics::device &device)
  : shading_system(device, utils::shading_type::WIRE_FRUSTUM)
{
  pipeline_layout_ = create_pipeline_layout<wire_frustum_push_constant>(
    static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
    std::vector<VkDescriptorSetLayout>{ get_global_desc_set_layout() }
  );

  auto pipeline_config_info = graphics::pipeline::default_pipeline_config_info();
  pipeline_config_info.binding_descriptions   = graphics::vertex::get_binding_descriptions();
  pipeline_config_info.attribute_descriptions = graphics::vertex::get_attribute_descriptions();

  pipeline_ = create_pipeline(
    pipeline_layout_,
    get_default_render_pass(),
    "/modules/graphics/shader/spv/",
    { "wire_frustum_shader.vert.spv", "wire_frustum_shader.frag.spv" },
    { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
    pipeline_config_info
  );
}

void wire_frustum_shading_system::render(const utils::frame_info &frame_info)
{
  auto command_buffer = frame_info.command_buffer;
  pipeline_->bind(command_buffer);

  vkCmdBindDescriptorSets(
    command_buffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline_layout_,
    0,
    1,
    &frame_info.global_descriptor_set,
    0,
    nullptr
  );

  for (auto& target : render_target_map_) {
    auto obj = dynamic_cast<game::wire_frame_frustum_component*>(&target.second);
    wire_frustum_push_constant push{};
    push.model_mat = obj->get_transform_sp()->mat4().cast<float>();

    vkCmdPushConstants(
      command_buffer,
      pipeline_layout_,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(wire_frustum_push_constant),
      &push
    );

    obj->get_frustum_mesh_sp()->bind(command_buffer);
    obj->get_frustum_mesh_sp()->draw(command_buffer);
  }
}

} // namespace hnll::game