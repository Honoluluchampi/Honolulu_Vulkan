#pragma once 

// hnll
#include <utils/utils.hpp>

// libs
#include <eigen3/Eigen/Dense>
#include <vulkan/vulkan.h>

namespace hnll::graphics {

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
  Eigen::Matrix4f projection   = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f view         = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f inverse_view = Eigen::Matrix4f::Identity();
  // point light
  glm::vec4 ambient_light_color{1.f, 1.f, 1.f, .02f}; // w is light intensity
  point_light point_lights[MAX_LIGHTS];
  int lights_count;
};

struct frame_info
{
  int frame_index;
  VkCommandBuffer command_buffer;
  VkDescriptorSet global_descriptor_set;
  utils::frustum_info* frustum_info;
};
} // namespace hnll::graphics