#include <hve_model.hpp>

//std
#include <cstring>

namespace hve {

HveModel::HveModel(HveDevice& device, const std::vector<Vertex> &vertices) : hveDevice_m{device}
{
  createVertexBuffers(vertices);
}

HveModel::~HveModel()
{
  vkDestroyBuffer(hveDevice_m.device(), vertexBuffer_m, nullptr);
  vkFreeMemory(hveDevice_m.device(), vertexBufferMemory_m, nullptr);
}

void HveModel::createVertexBuffers(const std::vector<Vertex> &vertices)
{
  vertexCount_m = static_cast<uint32_t>(vertices.size());
  // vertexCount must be larger than 3 (triangle) 
  // use a host visible buffer as temporary buffer, use a device local buffer as actual vertex buffer
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount_m;
  hveDevice_m.createBuffer(
    bufferSize,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    vertexBuffer_m,
    vertexBufferMemory_m
  );
  // filling in the data
  void *data;
  vkMapMemory(hveDevice_m.device(), vertexBufferMemory_m, 0, bufferSize, 0, &data);
  std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(hveDevice_m.device(), vertexBufferMemory_m);
}

void HveModel::draw(VkCommandBuffer commandBuffer)
{
  vkCmdDraw(commandBuffer, vertexCount_m, 1, 0, 0);
}

void HveModel::bind(VkCommandBuffer commandBuffer)
{
  VkBuffer buffers[] = {vertexBuffer_m};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

std::vector<VkVertexInputBindingDescription> HveModel::Vertex::getBindingDescriptions()
{
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  // per-vertex data is packed together in one array, so the index of the 
  // binding in the array is always 0
  bindingDescriptions[0].binding   = 0;
  // number of bytes from one entry to the next
  bindingDescriptions[0].stride    = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescriptions;
}
std::vector<VkVertexInputAttributeDescription> HveModel::Vertex::getAttributeDescriptions()
{
  // how to extract a vertex attribute from a chunk of vertex data
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
  attributeDescriptions[0].binding = 0;
  // the location directive of the input in the vertex shader.
  attributeDescriptions[0].location = 0;
  // same enumeration as color formats
  // 2-component vector of 32-bit float
  // for 2d
  // attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
  // for 3d
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  // the offset parameter specifies the number of bytes since the start of the per-vertex data to read from
  // the binding loads one Vertex at a time and the position attribute is at an offset of 0 bytes
  // Vertex::glm::vec2 position_m
  attributeDescriptions[0].offset = offsetof(Vertex, position_m);

  // color
  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Vertex, color_m);
  return attributeDescriptions;
}
} // namespace hveo