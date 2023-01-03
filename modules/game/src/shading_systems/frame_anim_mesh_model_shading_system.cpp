// hnll
#include <game/shading_systems/frame_anim_mesh_model_shading_system.hpp>
#include <graphics/frame_anim_mesh_model.hpp>

namespace hnll::game {

struct frame_anim_push_constant
{
  mat4 model_matrix;
  mat4 normal_matrix;
};

u_ptr<frame_anim_mesh_model_shading_system> frame_anim_mesh_model_shading_system::create(graphics::device& device)
{ return std::make_unique<frame_anim_mesh_model_shading_system>(device); }

frame_anim_mesh_model_shading_system::frame_anim_mesh_model_shading_system(graphics::device &device)
  : shading_system(device, utils::shading_type::FRAME_ANIM_MESH)
{
  pipeline_layout_ = create_pipeline_layout<frame_anim_push_constant>(
    static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
    // TODO : update desc_set_layout
    std::vector<VkDescriptorSetLayout>{ get_global_desc_set_layout() }
  );

  auto pipeline_config_info = graphics::pipeline::default_pipeline_config_info();
  pipeline_config_info.binding_descriptions   = graphics::frame_anim_mesh_model::get_binding_descriptions();
  pipeline_config_info.attribute_descriptions = graphics::frame_anim_mesh_model::get_attributes_descriptions();

  pipeline_ = create_pipeline(
    pipeline_layout_,
    shading_system::get_default_render_pass(),
    "/modules/graphics/shader/spv/",
    { "frame_anim_mesh.vert.spv", "simple_shader.frag.spv" },
    { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
    pipeline_config_info
  );
}

void frame_anim_mesh_model_shading_system::render(const utils::frame_info& frame_info)
{
  pipeline_->bind(frame_info.command_buffer);


}
}