#pragma once 

// hnll
#include <graphics/camera.hpp>

// libs
#include <vulkan/vulkan.h>

namespace hnll {
namespace graphics {

// TODO : decrese this
#define MAX_LIGHTS 20

struct point_light
{
  glm::vec4 position{}; // ignore w
  glm::vec4 color{}; // w is intensity
};

// global uniform buffer object
struct global_ubo
{
  // check alignment rules
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  // point light
  glm::vec4 ambient_light_color{1.f, 1.f, 1.f, .02f}; // w is light intensity
  point_light point_lights[MAX_LIGHTS];
  int lights_count;
};

struct frame_info
{
  int frame_index;
  VkCommandBuffer command_buffer;
  VkDescriptorSet global_discriptor_set;
};

} // namespace graphics
} // namesapce hnll