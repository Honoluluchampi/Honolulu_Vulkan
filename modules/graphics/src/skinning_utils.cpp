// hnll
#include <graphics/skinning_utils.hpp>
#include <graphics/device.hpp>
#include <graphics/buffer.hpp>
#include <graphics/descriptor_set_layout.hpp>

namespace hnll::graphics {

skinning_utils::mesh_group::mesh_group(device& device) : device_(device) {}

void skinning_utils::mesh_group::build_desc()
{
  desc_buffer_ = buffer::create(
    device_,
    sizeof(uniform_block),
    1,
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    &block
  );
}

void skinning_utils::mesh_group::update_desc_buffer()
{
  desc_buffer_->write_to_buffer(&block);
}

mat4 skinning_utils::node::get_local_matrix()
{
  return (Eigen::Translation3f(translation) * Eigen::Scaling(scale) * rotation).matrix();
}

mat4 skinning_utils::node::get_matrix()
{
  auto mat = get_local_matrix();
  auto par = parent;
  while (par) {
    mat = par->get_local_matrix() * mat;
    par = par->parent;
  }
  return mat;
}

void skinning_utils::node::update()
{
  if (mesh_group_) {
    auto mat = get_matrix();
    if (skin_) {
      mesh_group_->block.matrix = mat;
      // update joint matrices
      auto inv_mat = mat.inverse();
      size_t num_joints = std::min(static_cast<uint32_t>(skin_->joints.size()), MAX_JOINTS_NUM);
      for (size_t i = 0; i < num_joints; i++) {
        auto& joint_node = skin_->joints[i];
        mat4  joint_mat  = joint_node->get_matrix() * skin_->inv_bind_matrices[i];
        joint_mat = inv_mat * joint_mat;
        mesh_group_->block.joint_matrices[i] = joint_mat;
      }
      mesh_group_->block.joint_count = static_cast<float>(num_joints);
    }
  }
  mesh_group_->update_desc_buffer();

  for (auto& child : children) {
    child->update();
  }
}

std::vector<VkVertexInputBindingDescription>   skinning_utils::vertex::get_binding_descriptions()
{
  std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
  binding_descriptions[0].binding = 0;
  binding_descriptions[0].stride = sizeof(skinning_utils::vertex);
  binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> skinning_utils::vertex::get_attribute_descriptions()
{
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
  attribute_descriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(skinning_utils::vertex, position)});
  attribute_descriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(skinning_utils::vertex, normal)});
  attribute_descriptions.push_back({2, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(skinning_utils::vertex, tex_coord_0)});
  attribute_descriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(skinning_utils::vertex, tex_coord_1)});
  attribute_descriptions.push_back({4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(skinning_utils::vertex, color)});
  attribute_descriptions.push_back({5, 0, VK_FORMAT_R32G32B32A32_UINT,   offsetof(skinning_utils::vertex, joint_indices)});
  attribute_descriptions.push_back({6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(skinning_utils::vertex, joint_weights)});

  return attribute_descriptions;
}

}