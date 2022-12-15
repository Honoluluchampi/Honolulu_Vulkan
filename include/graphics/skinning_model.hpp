#pragma once

// lib
#include <eigen3/Eigen/Dense>

// std
#include <memory>

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

// forward declaration
class device;
class buffer;

class skinning_model
{
  public:
    // associated class -----------------------------------------------------

    class node
    {
      public:
        node()  = default;
        ~node() = default;

        // getter
        std::string get_name() const { return name_; }
        vec3 get_translation() const { return translation_; }
        quat get_rotation()    const { return rotation_; }
        vec3 get_scale()       const { return scale_; }
        std::vector<int> get_children() const { return children_; }

      private:
        std::string name_ = "";

        vec3 translation_ = { 0.f, 0.f, 0.f };
        quat rotation_    = { 1.f, 0.f, 0.f, 0.f };
        vec3 scale_       = { 1.f, 1.f, 1.f };
        mat4 local_mat_   = Eigen::Matrix4f::Identity();
        mat4 world_mat_   = Eigen::Matrix4f::Identity();

        std::vector<int> children_;
        s_ptr<node>      parent_ = nullptr;

        int mesh_index = -1;

        friend class skinning_model;
    };

    struct mesh
    {
      uint32_t index_start = 0;
      uint32_t vertex_start = 0;
      uint32_t index_count = 0;
      uint32_t vertex_count = 0;
      uint32_t material_index = 0;
    };

    class mesh_group
    {
      public:
        int get_node() const { return node_index_; }
        std::vector<mesh> get_meshes() const { return meshes_; }

      private:
        int node_index_;
        std::vector<mesh> meshes_;
        friend class skinning_model;
    };

    class skin
    {
      std::string name_;
      std::vector<int> joints_;
      std::vector<mat4> inv_bind_matrices_;
      friend class skinning_model;
    };

    class material
    {
      public:
        // getter
        std::string get_name()          const { return name_; }
        int         get_texture_index() const { return texture_index_; }
        vec3        get_diffuse_color() const { return diffuse_color_; }

      private:
        std::string name_ = "";
        int texture_index_ = -1;
        vec3 diffuse_color_ = { 1.f, 1.f, 1.f };

        friend class skinning_model;
    };

    struct image_info
    {
      std::vector<uint8_t> image_buffer;
      std::string filepath;
    };

    struct texture_info { int image_index; };

    // ---------------------------------------------------------------------

    skinning_model()  = default;
    ~skinning_model() = default;

    void load_from_gltf(const std::string& filepath, device& device);


  private:
    // associated struct ----------------------------------------------------
    struct vertex_attribute_visitor
    {
      std::vector<uint32_t> index_buffer;
      std::vector<vec3>     position_buffer;
      std::vector<vec3>     normal_buffer_;
      std::vector<vec2>     tex_coord_buffer_;

      std::vector<uvec4> joint_buffer;
      std::vector<vec4>  weight_buffer;
    };

    struct vertex_attribute_buffer
    {
      u_ptr<buffer> position_buffer;
      u_ptr<buffer> normal_buffer;
      u_ptr<buffer> tex_coord_buffer;
      u_ptr<buffer> joint_index_buffer;
      u_ptr<buffer> joint_weight_buffer;
    };

    struct skin_info
    {
      std::string name;
      std::vector<int> joints;
      std::vector<mat4> inv_bind_matrices;
      uint32_t skin_vertex_count;
    };
    // ---------------------------------------------------------------------

    void load_node(const tinygltf::Model& model);
    void load_mesh(const tinygltf::Model& model, vertex_attribute_visitor& visitor);
    void load_skin(const tinygltf::Model& model);
    void load_material(const tinygltf::Model& model);

    vertex_attribute_buffer vertex_attribute_buffer_;
    u_ptr<buffer> index_buffer_;

    std::vector<mesh_group> mesh_groups_;
    std::vector<material> materials_;
    std::vector<s_ptr<node>> nodes_;
    std::vector<int> root_nodes_;

    std::vector<image_info> images_;
    std::vector<texture_info> textures_;
};

}} // hnll::graphics