#pragma once

// libs
#include <eigen3/Eigen/Dense>
#include <vulkan/vulkan.h>

#define MAX_LIGHTS 20

namespace hnll::utils {

// rendering order matters for alpha blending
// solid object should be drawn first, then transparent object should be drawn after that
enum class rendering_type : uint32_t
{
  MESH         = 10,
  MESHLET      = 20,
  POINT_LIGHT  = 40,
  LINE         = 0,
  WIRE_FRUSTUM = 30,
  GRID         = 50
};

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

struct viewer_info
{
  Eigen::Matrix4f projection   = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f view         = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f inverse_view = Eigen::Matrix4f::Identity();
};

struct frustum_info
{
  Eigen::Vector3f camera_position;
  Eigen::Vector3f near_position;
  Eigen::Vector3f far_position;
  Eigen::Vector3f top_n;
  Eigen::Vector3f bottom_n;
  Eigen::Vector3f right_n;
  Eigen::Vector3f left_n;
  Eigen::Vector3f near_n;
  Eigen::Vector3f far_n;
};


struct frame_info
{
  int frame_index;
  VkCommandBuffer command_buffer;
  VkDescriptorSet global_descriptor_set;
};
} // namespace hnll::utils