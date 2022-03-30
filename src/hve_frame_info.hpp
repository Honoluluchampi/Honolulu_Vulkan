#pragma once 

#include <hve_camera.hpp>
#include <hve_game_object.hpp>

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
  // enable renderer to access all active gameObjects
  HveGameObject::Map &gameObjects_m;
};
} // namesapce hve