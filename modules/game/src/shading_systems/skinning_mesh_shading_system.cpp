// hnll
#include <game/shading_systems/skinning_mesh_model_shading_system.hpp>
#include <game/components/skinning_mesh_component.hpp>

namespace hnll::game {

using mat4 = Eigen::Matrix4f;

struct skinning_mesh_push_constant
{
  mat4 model_matrix;
  mat4 normal_matrix;
};

u_ptr<skinning_mesh_model_shading_system> skinning_mesh_model_shading_system::create(graphics::device& device)
{ return std::make_unique<skinning_mesh_model_shading_system>(device); }

skinning_mesh_model_shading_system::skinning_mesh_model_shading_system(graphics::device &device)
 : shading_system(device, utils::shading_type::SKINNING_MESH)
{
  pipeline_layout_ = create_pipeline_layout<skinning_mesh_push_constant>(
    static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
    std::vector<VkDescriptorSetLayout>{ get_global_desc_set_layout() }
  );

  pipeline_ = create_pipeline(
    pipeline_layout_,
    shading_system::get_default_render_pass(),
    "/modules/graphics/shader/spv/",
    { "skinning_mesh.vert.spv", "simple_shader.frag.spv" },
    { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT },
    graphics::pipeline::default_pipeline_config_info()
  );
}

void skinning_mesh_model_shading_system::setup_node_desc_set()
{
  node_desc_buffers_.resize(graphics::swap_chain::MAX_FRAMES_IN_FLIGHT);
  node_desc_sets_.resize(graphics::swap_chain::MAX_FRAMES_IN_FLIGHT);

  node_desc_pool_ = graphics::descriptor_pool::builder(device_)
    .set_max_sets(graphics::swap_chain::MAX_FRAMES_IN_FLIGHT)
    .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, graphics::swap_chain::MAX_FRAMES_IN_FLIGHT)
    .build();

  for (int i = 0; i < node_desc_buffers_.size(); i++) {
    node_desc_buffers_[i] = std::make_unique<graphics::buffer>(
      device_,
      sizeof(utils::frustum_info),
      1,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );
    node_desc_buffers_[i]->map();
  }

  node_desc_layout_ = graphics::descriptor_set_layout::builder(device_)
    .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_TASK_BIT_NV)
    .build();

  for (int i = 0; i < node_desc_sets_.size(); i++) {
    auto buffer_info = node_desc_buffers_[i]->descriptor_info();
    graphics::descriptor_writer(*node_desc_layout_, *node_desc_pool_)
      .write_buffer(0, &buffer_info)
      .build(node_desc_sets_[i]);
  }
}

void skinning_mesh_model_shading_system::render(const utils::frame_info& frame_info)
{
  pipeline_->bind(frame_info.command_buffer);

  vkCmdBindDescriptorSets(
    frame_info.command_buffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline_layout_,
    0,
    1,
    &frame_info.global_descriptor_set,
    0,
    nullptr
  );

  for (auto& target : render_target_map_) {
    auto obj = dynamic_cast<skinning_mesh_component*>(&target.second);

    skinning_mesh_push_constant push{};
    push.model_matrix  = obj->get_transform().mat4().cast<float>();
    push.normal_matrix = obj->get_transform().normal_matrix().cast<float>();

    vkCmdPushConstants(
      frame_info.command_buffer,
      pipeline_layout_,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(skinning_mesh_push_constant),
      &push
    );

    obj->get_model().bind(frame_info.command_buffer);
    obj->get_model().draw(frame_info.command_buffer);
  }
}

} // namespace hnll::game