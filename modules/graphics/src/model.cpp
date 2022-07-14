// hnll
#include <graphics/model.hpp>

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cstring>
#include <iostream>
#include <unordered_map>

namespace std {

template <>
struct hash<hnll::HveModel::Vertex>
{
  size_t operator() (hnll::HveModel::Vertex const &vertex) const
  {
    // stores final hash value
    size_t seed = 0;
    hnll::hashCombine(seed, vertex.position_m, vertex.color_m, vertex.normal_m, vertex.uv_m);
    return seed;
  }
};
}

namespace hnll {

HveModel::HveModel(HveDevice& device, const HveModel::Builder &builder) : hveDevice_m{device}
{
  createVertexBuffers(builder.vertices_m);
  createIndexBuffers(builder.indices_m);
}

HveModel::~HveModel()
{
  // buffers wille be freed in dotr of Hvebuffer
}

std::shared_ptr<HveModel> HveModel::createModelFromFile(HveDevice &device, const std::string &filename)
{
  Builder builder;
  builder.loadModel(filename);
  std::cout << filename << " Vertex count: " << builder.vertices_m.size() << "\n";
  return std::make_shared<HveModel>(device, builder);
}

void HveModel::createVertexBuffers(const std::vector<Vertex> &vertices)
{
  // vertexCount must be larger than 3 (triangle) 
  // use a host visible buffer as temporary buffer, use a device local buffer as actual vertex buffer
  vertexCount_m = static_cast<uint32_t>(vertices.size());
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount_m;
  uint32_t vertexSize = sizeof(vertices[0]);

  // staging buffer creation
  HveBuffer stagingBuffer {
    hveDevice_m,
    vertexSize, // for calculating alignment
    vertexCount_m, // same as above
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // usage
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT // property
  };
  // mapping the data to the buffer
  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)vertices.data());

  // vertex buffer creation
  vertexBuffer_m = std::make_unique<HveBuffer>(
    hveDevice_m,
    vertexSize, // for calculating alignment
    vertexCount_m, // same as above
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, // usage
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT// property
  );
  // copy the data from staging buffer to the vertex buffer
  hveDevice_m.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer_m->getBuffer(), bufferSize);
  // staging buffer is automatically freed in the dtor 
}

void HveModel::createIndexBuffers(const std::vector<uint32_t> &indices)
{
  indexCount_m = static_cast<uint32_t>(indices.size());
  // if there is no index, nothing to do
  hasIndexBuffer_m = indexCount_m > 0;
  if (!hasIndexBuffer_m) return;

  VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount_m;
  uint32_t indexSize = sizeof(indices[0]);

  // copy the data to the staging buffer
  HveBuffer stagingBuffer {
    hveDevice_m,
    indexSize,
    indexCount_m,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)indices.data());

  indexBuffer_m = std::make_unique<HveBuffer> (
    hveDevice_m,
    indexSize,
    indexCount_m,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT // optimal type of the memory type
  );

  // copy the data from staging buffer to the vertex buffer
  hveDevice_m.copyBuffer(stagingBuffer.getBuffer(), indexBuffer_m->getBuffer(), bufferSize);
}

void HveModel::draw(VkCommandBuffer commandBuffer)
{
  if (hasIndexBuffer_m)
    vkCmdDrawIndexed(commandBuffer, indexCount_m, 1, 0, 0, 0);
  else 
    vkCmdDraw(commandBuffer, vertexCount_m, 1, 0, 0);
}

void HveModel::bind(VkCommandBuffer commandBuffer)
{
  VkBuffer buffers[] = {vertexBuffer_m->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  // last parameter should be same as the type of the Build::indices
  if (hasIndexBuffer_m) 
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer_m->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
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
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

  // location, binding, format, offset
  attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position_m)});
  attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color_m)});
  attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal_m)});
  attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv_m)});

  return attributeDescriptions;
}

void HveModel::Builder::loadModel(const std::string& filename)
{
  // loader
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
    throw std::runtime_error(warn + err);

  vertices_m.clear();
  indices_m.clear();

  std::unordered_map<Vertex, uint32_t> uniqueVertices{};


  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Vertex vertex{};
      // copy the vertex
      if (index.vertex_index >= 0) {
        vertex.position_m = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2]
        };
        // color support
        vertex.color_m = {
          attrib.colors[3 * index.vertex_index + 0],
          attrib.colors[3 * index.vertex_index + 1],
          attrib.colors[3 * index.vertex_index + 2]
        };
      }
      // copy the normal
      if (index.vertex_index >= 0) {
        vertex.normal_m = {
          attrib.normals[3 * index.normal_index + 0],
          attrib.normals[3 * index.normal_index + 1],
          attrib.normals[3 * index.normal_index + 2]
        };
      }
      // copy the texture coordinate
      if (index.vertex_index >= 0) {
        vertex.uv_m = {
          attrib.vertices[2 * index.texcoord_index + 0],
          attrib.vertices[2 * index.texcoord_index + 1]
        };
      }
      // if vertex is a new vertex
      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices_m.size());
        vertices_m.push_back(std::move(vertex));
      }
      indices_m.push_back(uniqueVertices[vertex]);
    }
  }
}
} // namespace hveo