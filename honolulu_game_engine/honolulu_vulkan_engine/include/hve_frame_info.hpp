#pragma once 

#include <hve_camera.hpp>
#include <hve_game_object.hpp>
#include <hge_components/model_component.hpp>

// libs
#include <vulkan/vulkan.h>

namespace hnll {

#define MAX_LIGHTS 10

struct PointLight
{
  glm::vec4 position{}; // ignore w
  glm::vec4 color{}; // w is intensity
};

// global uniform buffer object
struct GlobalUbo
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
  float frameTime_m;
  VkCommandBuffer commandBuffer_m;
  HveCamera &camera_m;
  VkDescriptorSet globalDiscriptorSet_m;
  // to do : pass only active models, separate hveFrameInfo and hgeFrameInfo
  // enable renderer to access all active gameObjects
  std::unordered_map<unsigned int, std::shared_ptr<ModelComponent>>& modelMap_m;
};
} // namesapce hve