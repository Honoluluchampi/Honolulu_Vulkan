// hnll
#include <game/shading_systems/gltf_mesh_shading_system.hpp>

namespace hnll::game {

using mat4 = Eigen::Matrix4f;

struct gltf_mesh_push_constant
{
  mat4 dummy;
};

u_ptr<gltf_mesh_shading_system> gltf_mesh_shading_system::create(graphics::device& device)
{ return std::make_unique<gltf_mesh_shading_system>(device); }

gltf_mesh_shading_system::gltf_mesh_shading_system(graphics::device &device)
 : shading_system(device, utils::shading_type::GLTF_MESH)
{
  pipeline_layout_ = create_pipeline_layout<gltf_mesh_push_constant>(
    static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
    std::vector<VkDescriptorSetLayout>{ get_global_desc_set_layout() }
  );

  pipeline_ = create_pipeline(
    pipeline_layout_,
    shading_system::get_default_render_pass(),
    "/applications/gltf_animation/shader/spv/",
    { "tangent.vert.spv", "tangent_bc_mr_oc_em_no.frag.spv" },
    { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
    graphics::pipeline::default_pipeline_config_info()
  );
}

void gltf_mesh_shading_system::render(const utils::frame_info& frame_info)
{

}

} // namespace hnll::game