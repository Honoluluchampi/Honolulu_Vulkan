// hnll
#include <graphics/utils.hpp>

namespace hnll::graphics {

std::vector<VkVertexInputBindingDescription> vertex::get_binding_descriptions() {
  std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
  // per-vertex data is packed together in one array, so the index of the
  // binding in the array is always 0
  binding_descriptions[0].binding = 0;
  // number of bytes from one entry to the next
  binding_descriptions[0].stride = sizeof(vertex);
  binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> vertex::get_attribute_descriptions() {
  // how to extract a vertex attribute from a chunk of vertex data
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

  // location, binding, format, offset
  attribute_descriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, position)});
  attribute_descriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, color)});
  attribute_descriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, normal)});
  attribute_descriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, uv)});

  return attribute_descriptions;
}

} // namespace hnll::graphics