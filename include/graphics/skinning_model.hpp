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

    static u_ptr<skinning_model> create_from_gltf(const std::string& filepath, device& device);
    bool load_from_gltf(const std::string& filepath, device& device);

    // getter
    // buffer
    const buffer& get_position_buffer()     const { return *vertex_attribute_buffer_.position_buffer; }
    const buffer& get_normal_buffer()       const { return *vertex_attribute_buffer_.normal_buffer; }
    const buffer& get_tex_coord_buffer()    const { return *vertex_attribute_buffer_.tex_coord_buffer; }
    const buffer& get_joint_index_buffer()  const { return *vertex_attribute_buffer_.joint_index_buffer; }
    const buffer& get_joint_weight_buffer() const { return *vertex_attribute_buffer_.joint_weight_buffer; }
    const buffer& get_index_buffer()        const { return *index_buffer_; }

    // image and texture
    uint32_t get_texture_count() const { return static_cast<uint32_t>(textures_.size()); }
    uint32_t get_image_count()   const { return static_cast<uint32_t>(images_.size()); }
    std::vector<texture_info> get_textures() const { return textures_; }
    std::vector<image_info>   get_iamges()   const { return images_; }

    // node
    uint32_t get_node_count()         const { return static_cast<uint32_t>(nodes_.size()); }
    s_ptr<node> get_node(int index)   const { return nodes_[index]; }
    std::vector<int> get_root_nodes() const { return root_nodes_; }

    // material
    std::vector<material> get_materials() const { return materials_; }

    // mesh group
    uint32_t get_mesh_group_count() const { return static_cast<uint32_t>(mesh_groups_.size()); }
    std::vector<mesh_group> get_mesh_groups() const { return mesh_groups_; }

    // others
    bool is_skinned() const { return has_skin_; }

    std::vector<std::string> get_joint_node_names() const;

    std::vector<mat4> get_inv_bind_matrices() const;

    uint32_t get_skinned_vertex_count() const { return skin_info_.skin_vertex_count; }

  private:
    // associated struct ----------------------------------------------------
    struct vertex_attribute_visitor
    {
      std::vector<uint32_t> index_buffer;
      std::vector<vec3>     position_buffer;
      std::vector<vec3>     normal_buffer;
      std::vector<vec2>     tex_coord_buffer;

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

    bool has_skin_ = false;
    skin_info skin_info_;

    std::vector<image_info> images_;
    std::vector<texture_info> textures_;
};

}} // hnll::graphics