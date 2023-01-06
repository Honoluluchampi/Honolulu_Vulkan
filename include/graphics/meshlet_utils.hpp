#pragma once

// hnll
#include <utils/common_using.hpp>

namespace hnll::graphics {

constexpr uint32_t VERTEX_DESC_ID  = 0;
constexpr uint32_t MESHLET_DESC_ID = 1;
constexpr uint32_t DESC_SET_COUNT  = 2;
constexpr uint32_t MAX_VERTEX_PER_MESHLET = 64;
constexpr uint32_t MAX_INDEX_PER_MESHLET  = 378;
constexpr uint32_t MAX_MESHLET_COUNT = 1024;

struct meshlet
{
  uint32_t vertex_indices   [MAX_VERTEX_PER_MESHLET]; // indicates position in a vertex buffer
  uint32_t primitive_indices[MAX_INDEX_PER_MESHLET];
  uint32_t vertex_count;
  uint32_t index_count;
  // for frustum culling (for bounding sphere)
  alignas(16) vec3 center;
  float            radius;
  // for aabb
  // alignas(16) vec3 radius;
};

} // namespace hnll::graphics