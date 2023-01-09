// hnll
#include <graphics/frame_anim_utils.hpp>
#include <graphics/skinning_mesh_model.hpp>

namespace hnll::graphics {

Eigen::Matrix3f mat4to3(const mat4& original)
{
  Eigen::Matrix3f ret;
  ret << original(0, 0), original(0, 1), original(0, 2),
    original(1, 0), original(1, 1), original(1, 2),
    original(2, 0), original(2, 1), original(2, 2);
  return ret;
}

void extract_node_vertices(
  skinning_utils::node& node,
  std::vector<frame_anim_utils::dynamic_attributes>& buffer,
  const skinning_utils::builder& builder,
  std::vector<bool>& vertex_computed)
{
  if (node.mesh_group_ == nullptr)
    return;

  for (auto& mesh : node.mesh_group_->meshes) {
    for (int i = 0; i < mesh.index_count; i++) {
      uint32_t index = builder.index_buffer[i + mesh.first_index];
      // already calculated
      if (vertex_computed[index])
        continue;

      // compute vertex position and normal
      const auto& target = builder.vertex_buffer[index];
      vec3 new_position, new_normal;
      if (node.mesh_group_->block.joint_count > 0) {
        const auto &joint_matrices = node.mesh_group_->block.joint_matrices;
        mat4 skin_mat = target.joint_weights.x() * joint_matrices[static_cast<int>(target.joint_indices.x())]
                        + target.joint_weights.y() * joint_matrices[static_cast<int>(target.joint_indices.y())]
                        + target.joint_weights.z() * joint_matrices[static_cast<int>(target.joint_indices.z())]
                        + target.joint_weights.w() * joint_matrices[static_cast<int>(target.joint_indices.w())];
        mat4 transform_mat = node.mesh_group_->block.matrix * skin_mat;
        vec4 position = transform_mat * vec4{ target.position.x(), target.position.y(), target.position.z(), 1.f};
        new_position = vec3{ position.x(), position.y(), position.z() };
        new_normal = mat4to3(transform_mat) * target.normal;
      }
      else {
        vec4 position = node.mesh_group_->block.matrix * vec4{ target.position.x(), target.position.y(), target.position.z(), 1.f};
        new_position = vec3{ position.x(), position.y(), position.z() };
        new_normal = mat4to3(node.mesh_group_->block.matrix) * target.normal;
      }
      buffer[index] = { new_position, new_normal };
      vertex_computed[index] = true;
    }
  }
}

std::vector<frame_anim_utils::dynamic_attributes> frame_anim_utils::extract_dynamic_attributes(
  skinning_mesh_model& original,
  const skinning_utils::builder& builder)
{
  size_t vertex_count = builder.vertex_buffer.size();
  std::vector<frame_anim_utils::dynamic_attributes> new_dynamic_attribs(vertex_count);
  std::vector<bool> vertex_computed(vertex_count, false);

  for (auto& node : original.get_nodes()) {
    extract_node_vertices(*node, new_dynamic_attribs, builder, vertex_computed);
    for (auto& child : node->children) {
      extract_node_vertices(*child, new_dynamic_attribs, builder, vertex_computed);
    }
  }

  return new_dynamic_attribs;
}

} // hnll::graphics