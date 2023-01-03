// hnll
#include <game/shading_systems/skinning_mesh_model_shading_system.hpp>
#include <game/components/skinning_mesh_component.hpp>

namespace hnll {
namespace graphics {

void graphics::skinning_mesh_model::setup_desc_set_layout(device& device)
{
  desc_set_layout_ = descriptor_set_layout::builder(device)
    .add_binding(
      0,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT
    )
    .build();
}
} // namespace graphics

namespace game {

using mat4 = Eigen::Matrix4f;

u_ptr<skinning_mesh_model_shading_system> skinning_mesh_model_shading_system::create(graphics::device& device)
{ return std::make_unique<skinning_mesh_model_shading_system>(device); }

skinning_mesh_model_shading_system::skinning_mesh_model_shading_system(graphics::device &device)
 : shading_system(device, utils::shading_type::SKINNING_MESH)
{
  graphics::skinning_mesh_model::setup_desc_set_layout(device_);

  pipeline_layout_ = create_pipeline_layout<graphics::skinning_mesh_push_constant>(
    static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
    std::vector<VkDescriptorSetLayout>{ get_global_desc_set_layout(), graphics::skinning_mesh_model::get_desc_set_layout() }
  );

  auto pipeline_config_info = graphics::pipeline::default_pipeline_config_info();
  pipeline_config_info.binding_descriptions   = graphics::skinning_utils::vertex::get_binding_descriptions();
  pipeline_config_info.attribute_descriptions = graphics::skinning_utils::vertex::get_attribute_descriptions();

  pipeline_ = create_pipeline(
    pipeline_layout_,
    shading_system::get_default_render_pass(),
    "/modules/graphics/shader/spv/",
    { "skinning_mesh.vert.spv", "simple_shader.frag.spv" },
    { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
    pipeline_config_info
  );
}

skinning_mesh_model_shading_system::~skinning_mesh_model_shading_system()
{
  graphics::skinning_mesh_model::erase_desc_set_layout();
}

void skinning_mesh_model_shading_system::render(const utils::frame_info& frame_info)
{
  pipeline_->bind(frame_info.command_buffer);

  static float animation_timer = 0.f;
  animation_timer += 0.001f;
  while (animation_timer > 0.8f) {
    animation_timer -= 0.8f;
  }
  for (auto& target : render_target_map_) {
    auto obj = dynamic_cast<skinning_mesh_component*>(&target.second);

    obj->get_model().update_animation(0, animation_timer);

    graphics::skinning_mesh_push_constant push{};
    push.model_matrix  = obj->get_transform().mat4().cast<float>();
    push.normal_matrix = obj->get_transform().normal_matrix().cast<float>();

    obj->get_model().bind(frame_info.command_buffer, frame_info.global_descriptor_set, pipeline_layout_);
    obj->get_model().draw(frame_info.command_buffer, frame_info.global_descriptor_set, pipeline_layout_, push);
  }
}

}} // namespace hnll::game