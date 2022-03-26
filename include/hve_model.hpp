#pragma once

#include <hve_device.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace hve {

class HveModel
{
  public:

    struct Vertex
    {
      glm::vec3 position_m;
      glm::vec3 color_m;
      // return a description compatible with the shader
      static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
      static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    struct Builder
    {
      // copied to the vertex buffer and index buffer
      std::vector<Vertex> vertices_m{};
      std::vector<uint32_t> indices_m{};
    };

    HveModel(HveDevice& device, const HveModel::Builder &builder);
    ~HveModel();

    HveModel(const HveModel &) = delete;
    HveModel& operator=(const HveModel &) = delete;

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

  private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t> &indices);

    HveDevice& hveDevice_m;
    // separating buffer and its assigned memory enables programmer to manage memory manually
    VkBuffer vertexBuffer_m;
    VkDeviceMemory vertexBufferMemory_m;
    uint32_t vertexCount_m;

    bool hasIndexBuffer_m = false;
    VkBuffer indexBuffer_m;
    VkDeviceMemory indexBufferMemory_m;
    uint32_t indexCount_m;
};
} // namespace hve