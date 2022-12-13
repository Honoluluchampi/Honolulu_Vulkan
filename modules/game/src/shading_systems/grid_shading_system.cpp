// hnll
#include <game/shading_systems/grid_shading_system.hpp>

namespace hnll::game {

u_ptr<grid_shading_system> create(graphics::device& device)
{ return std::make_unique<grid_shading_system>(device); }

grid_shading_system::grid_shading_system(graphics::device& device)
 : shading_system(device, utils::shading_type::GRID)
{
  pipeline_layout_ = create_pipeline_layout<no_push_constant>(
    static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
    std::vector<VkDescriptorSetLayout>{ get_global_desc_set_layout() }
  );

  pipeline_ = create_pipeline(
    pipeline_layout_,
    shading_system::get_default_render_pass(),
    "/modules/graphics/shader/spv/",
    { "grid_shader.vert.spv", "grid_shader.frag.spv" },
    { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT }
  );
}

} // namespace hnll::game