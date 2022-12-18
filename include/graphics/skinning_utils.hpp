#pragma once

// std
#include <memory>

// lib
#include <eigen3/Eigen/Dense>

// forward declaration
namespace tinygltf {
  class Node;
  class Model;
  struct Mesh;
}

namespace hnll {

using vec2  = Eigen::Vector2f;
using vec3  = Eigen::Vector3f;
using vec4  = Eigen::Vector4f;
using uvec4 = Eigen::Matrix<unsigned, 4, 1>;
using mat4  = Eigen::Matrix4f;
using quat  = Eigen::Quaternionf;
template<class T> using u_ptr = std::unique_ptr<T>;
template<class T> using s_ptr = std::shared_ptr<T>;

namespace graphics {

namespace skinning_utils
{

struct vertex
{
  alignas(16) vec3 position;
  alignas(16) vec3 normal;
  vec2  tex_coord;
  uvec4 joint_indices;
  vec4  joint_weights;
};

struct node
{
    node()  = default;
    ~node() = default;

    std::string name = "";

    vec3 translation = { 0.f, 0.f, 0.f };
    quat rotation    = { 1.f, 0.f, 0.f, 0.f };
    vec3 scale       = { 1.f, 1.f, 1.f };
    mat4 local_mat   = Eigen::Matrix4f::Identity();
    mat4 world_mat   = Eigen::Matrix4f::Identity();

    std::vector<int> children;
    s_ptr<node>      parent = nullptr;

    int mesh_index = -1;
};

struct mesh
{
  uint32_t index_start    = 0;
  uint32_t vertex_start   = 0;
  uint32_t index_count    = 0;
  uint32_t vertex_count   = 0;
  uint32_t material_index = 0;
};

struct mesh_group
{
    int node_index;
    std::vector<mesh> meshes;
};

struct skin
{
    std::string name;
    std::vector<int> joints;
    std::vector<mat4> inv_bind_matrices;
};

struct material
{
    std::string name = "";
    int texture_index = -1;
    vec3 diffuse_color = { 1.f, 1.f, 1.f };
};

struct image_info
{
  std::vector<uint8_t> image_buffer;
  std::string filepath;
};

struct texture_info { int image_index; };

struct skinning_model_builder
{
  std::vector<uint32_t> index_buffer;
  std::vector<vec3>     position_buffer;
  std::vector<vec3>     normal_buffer;
  std::vector<vec2>     tex_coord_buffer;

  std::vector<uvec4> joint_buffer;
  std::vector<vec4>  weight_buffer;
};

struct skin_info
{
  std::string name;
  std::vector<int> joints;
  std::vector<mat4> inv_bind_matrices;
  uint32_t skin_vertex_count;
};

}  // namespace skinning_utils
}} // namespace hnll::graphics