// hnll
#include <game/shading_systems/meshlet_model_shading_system.hpp>
#include <utils/common_using.hpp>
#include <graphics/descriptor_set_layout.hpp>


namespace hnll::game {

struct meshlet_push_constant
{
  mat4 model_matrix  = mat4::Identity();
  mat4 normal_matrix = mat4::Identity();
};

u_ptr<meshlet_model_shading_system> meshlet_model_shading_system::create(graphics::device &device)
{ return std::make_unique<meshlet_model_shading_system>(device); }

void meshlet_model_shading_system::setup_task_desc()
{

}

meshlet_model_shading_system::meshlet_model_shading_system(graphics::device &device)
  : shading_system(device, utils::shading_type::MESHLET)
{
  setup_task_desc();
  pipeline_layout_ = create_pipeline_layout<meshlet_push_constant>(
    static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_TASK_BIT_NV | VK_SHADER_STAGE_MESH_BIT_NV | VK_SHADER_STAGE_FRAGMENT_BIT),
    std::vector<VkDescriptorSetLayout>{get_global_desc_set_layout() }
  );

  auto pipeline_config_info = graphics::pipeline::default_pipeline_config_info();
  pipeline_config_info.binding_descriptions.clear();
  pipeline_config_info.attribute_descriptions.clear();

  pipeline_ = create_pipeline(
    pipeline_layout_,
    shading_system::get_default_render_pass(),
    "/applications/mesh_shader/introduction/shaders/spv/",
    { "simple_meshlet.task.glsl.spv", "simple_meshlet.mesh.glsl.spv", "simple_meshlet.frag.glsl.spv" },
    { VK_SHADER_STAGE_TASK_BIT_NV, VK_SHADER_STAGE_MESH_BIT_NV, VK_SHADER_STAGE_FRAGMENT_BIT },
    pipeline_config_info
  );
}

void meshlet_model_shading_system::render(const utils::frame_info &frame_info)
{

}

} // namespace hnll::game