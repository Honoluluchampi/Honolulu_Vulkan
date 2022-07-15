#pragma once 

// hnll
#include <graphics/camera.hpp>

// libs
#include <vulkan/vulkan.h>

namespace hnll {
namespace graphics {

// TODO : decrese this
#define MAX_LIGHTS 20

struct PointLight
{
  glm::vec4 position{}; // ignore w
  glm::vec4 color{}; // w is intensity
};

// global uniform buffer object
struct global_ubo
{
  // check alignment rules
  glm::mat4 projection_m{1.f};
  glm::mat4 view_m{1.f};
  // point light
  glm::vec4 ambientLightColor_m{1.f, 1.f, 1.f, .02f}; // w is light intensity
  PointLight pointLights[MAX_LIGHTS];
  int numLights;
};

struct FrameInfo
{
  int frameIndex_m;
  VkCommandBuffer commandBuffer_m;
  VkDescriptorSet globalDiscriptorSet_m;
};

} // namespace graphics
} // namesapce hve