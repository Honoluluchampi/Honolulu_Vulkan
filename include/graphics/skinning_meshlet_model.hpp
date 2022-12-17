#pragma once

#include <graphics/skinning_utils.hpp>

namespace hnll {
namespace graphics {

// forward declaration
class device;
class buffer;

class skinning_meshlet_model
{
  public:
    skinning_meshlet_model()  = default;
    ~skinning_meshlet_model() = default;

    static u_ptr<skinning_meshlet_model> create_from_gltf(const std::string& filepath, device& device);
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
    std::vector<skinning_utils::texture_info> get_textures() const { return textures_; }
    std::vector<skinning_utils::image_info>   get_images()   const { return images_; }

    // node
    uint32_t get_node_count()         const { return static_cast<uint32_t>(nodes_.size()); }
    s_ptr<skinning_utils::node> get_node(int index)   const { return nodes_[index]; }
    std::vector<int> get_root_nodes() const { return root_nodes_; }

    // material
    std::vector<skinning_utils::material> get_materials() const { return materials_; }

    // mesh group
    uint32_t get_mesh_group_count() const { return static_cast<uint32_t>(mesh_groups_.size()); }
    std::vector<skinning_utils::mesh_group> get_mesh_groups() const { return mesh_groups_; }

    // others
    bool is_skinned() const { return has_skin_; }

    std::vector<std::string> get_joint_node_names() const;

    std::vector<mat4> get_inv_bind_matrices() const;

    uint32_t get_skinned_vertex_count() const { return skin_info_.skin_vertex_count; }

  private:
    // for mesh shading
    struct vertex_attribute_buffer
    {
      u_ptr<buffer> position_buffer;
      u_ptr<buffer> normal_buffer;
      u_ptr<buffer> tex_coord_buffer;
      u_ptr<buffer> joint_index_buffer;
      u_ptr<buffer> joint_weight_buffer;
    };

    void load_node(const tinygltf::Model& model);
    void load_mesh(const tinygltf::Model& model, skinning_utils::vertex_attribute_visitor& visitor);
    void load_skin(const tinygltf::Model& model);
    void load_material(const tinygltf::Model& model);

    // buffer
    vertex_attribute_buffer vertex_attribute_buffer_;
    u_ptr<buffer>                  index_buffer_;

    std::vector<skinning_utils::mesh_group>  mesh_groups_;
    std::vector<skinning_utils::material>    materials_;
    std::vector<s_ptr<skinning_utils::node>> nodes_;
    std::vector<int> root_nodes_;

    bool has_skin_ = false;
    skinning_utils::skin_info skin_info_;

    std::vector<skinning_utils::image_info>   images_;
    std::vector<skinning_utils::texture_info> textures_;
};

}} // hnll::graphics