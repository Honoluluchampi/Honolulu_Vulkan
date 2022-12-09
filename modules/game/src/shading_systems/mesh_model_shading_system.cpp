// hnll
#include <game/shading_systems/mesh_model_shading_system.hpp>
#include <game/components/mesh_component.hpp>

namespace hnll {

using mat4 = Eigen::Matrix4f;

namespace game {

// init static member of shading_system
VkDescriptorSetLayout shading_system::global_desc_set_layout_;

struct mesh_push_constant
{
  mat4 model_matrix;
  mat4 normal_matrix;
};

VkDescriptorSetLayout layout;

mesh_model_shading_system::mesh_model_shading_system(graphics::device &device)
 : shading_system(device, utils::rendering_type::MESH)
{
  create_pipeline_layout(
    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    std::vector<VkDescriptorSetLayout>{ get_global_desc_set_layout() }
  );
}

void mesh_model_shading_system::render(const utils::frame_info& frame_info)
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
    auto obj = dynamic_cast<mesh_component*>(target.second.get());

    mesh_push_constant push{};
    push.model_matrix  = obj->get_transform().mat4().cast<float>();
    push.normal_matrix = obj->get_transform().normal_matrix().cast<float>();

    vkCmdPushConstants(
      frame_info.command_buffer,
      pipeline_layout_,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(mesh_push_constant),
      &push
    );

    obj->get_model().bind(frame_info.command_buffer);
    obj->get_model().draw(frame_info.command_buffer);
  }
}

}} // namespace hnll::game