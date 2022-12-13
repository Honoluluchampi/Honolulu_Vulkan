// hnll
#include <game/shading_systems/grid_shading_system.hpp>

namespace hnll::game {

struct grid_constant { float height; };

u_ptr<grid_shading_system> grid_shading_system::create(graphics::device& device)
{ return std::make_unique<grid_shading_system>(device); }

grid_shading_system::grid_shading_system(graphics::device& device)
 : shading_system(device, utils::shading_type::GRID)
{
  pipeline_layout_ = create_pipeline_layout<grid_constant>(
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

void grid_shading_system::render(const utils::frame_info &frame_info)
{
  pipeline_->bind(frame_info.command_buffer);

  vkCmdBindDescriptorSets(
    frame_info.command_buffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline_layout_,
    0, 1,
    &frame_info.global_descriptor_set,
    0, nullptr
  );

  grid_constant push{};
  push.height = 0.f;
  vkCmdPushConstants(
    frame_info.command_buffer,
    pipeline_layout_,
    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    0,
    sizeof(grid_constant),
    &push
  );
  vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
}

} // namespace hnll::game