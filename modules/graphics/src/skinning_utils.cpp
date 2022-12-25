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
  attribute_descriptions.push_back({2, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(skinning_utils::vertex, tex_coord)});
  attribute_descriptions.push_back({3, 0, VK_FORMAT_R32G32B32A32_UINT,   offsetof(skinning_utils::vertex, joint_indices)});
  attribute_descriptions.push_back({4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(skinning_utils::vertex, joint_weights)});

  return attribute_descriptions;
}

}