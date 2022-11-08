#pragma once

// std
#include <vector>

// lib
#include <eigen3/Eigen/Dense>
#include <vulkan/vulkan.h>

namespace hnll::graphics {

struct vertex
{
  alignas(16) Eigen::Vector3f position{};
  alignas(16) Eigen::Vector3f color{};
  alignas(16) Eigen::Vector3f normal{};
  // texture coordinates
  Eigen::Vector2f uv{};
  // return a description compatible with the shader
  static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
  static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();

  bool operator==(const vertex& other) const
  { return position == other.position && color == other.color && normal == other.normal && uv == other.uv; }
};

struct mesh_builder
{
  // copied to the vertex buffer and index buffer
  std::vector<vertex> vertices{};
  std::vector<uint32_t> indices{};

  void load_model(const std::string& filename);
};

} // namespace hnll::graphics