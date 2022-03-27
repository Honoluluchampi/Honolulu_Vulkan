#pragma once 

#include <hve_camera.hpp>

// libs
#include <vulkan/vulkan.h>

namespace hve {
struct FrameInfo
{
  int frameIndex_m;
  float frameTime_m;
  VkCommandBuffer commandBuffer_m;
  HveCamera &camera_m;
  VkDescriptorSet globalDiscriptorSet_m;
};
} // namesapce hve