#pragma once

// hnll
#include <graphics/skinning_utils.hpp>
#include <graphics/descriptor_set_layout.hpp>

// lib
#include <vulkan/vulkan.h>

namespace hnll {
namespace graphics {

// forward declaration
class device;
class buffer;
class descriptor_pool;
class descriptor_set_layout;

#define MAX_NUM_JOINTS 128

struct node_info {
  mat4  matrix = Eigen::Matrix4f::Identity();
  mat4  joint_matrices[MAX_NUM_JOINTS];
  float joint_count;
};

class skinning_mesh_model
{
  public:
    skinning_mesh_model()  = default;
    ~skinning_mesh_model() = default;

    void bind(VkCommandBuffer command_buffer, VkDescriptorSet global_desc_set, VkPipelineLayout pipeline_layout);
    void draw(VkCommandBuffer command_buffer);

    static u_ptr<skinning_mesh_model> create_from_gltf(const std::string& filepath, device& device);
    bool load_from_gltf(const std::string& filepath, device& device);

    // getter
    // buffer
    const buffer& get_vertex_buffer() const { return *vertex_buffer_; }
    const buffer& get_index_buffer()  const { return *index_buffer_; }

    // image and texture
    uint32_t get_texture_count() const { return static_cast<uint32_t>(textures_.size()); }
    uint32_t get_image_count()   const { return static_cast<uint32_t>(images_.size()); }
    std::vector<skinning_utils::texture_info> get_textures() const { return textures_; }
    std::vector<skinning_utils::image_info>   get_images()   const { return images_; }

    // node
    uint32_t         get_node_count() const { return static_cast<uint32_t>(nodes_.size()); }
    std::vector<int> get_root_nodes() const { return root_nodes_; }
    s_ptr<skinning_utils::node> get_node(int index) const { return nodes_[index]; }

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

    // for shading system
    static void setup_desc_set_layout(device& device);
    static VkDescriptorSetLayout get_desc_set_layout() { return desc_set_layout_->get_descriptor_set_layout(); }
    static void erase_desc_set_layout()                { desc_set_layout_.reset(); }

  private:

    void load_node(const tinygltf::Model& model);
    void load_mesh(const tinygltf::Model& model, skinning_utils::skinning_model_builder& visitor);
    void load_skin(const tinygltf::Model& model);
    void load_material(const tinygltf::Model& model);

    void setup_descs(device& device);
    void create_desc_pool(device& _device);
    void create_desc_buffers(device& _device);
    void create_desc_set_layouts(device& _device);
    void create_desc_sets();

    // buffer
    u_ptr<buffer> vertex_buffer_;
    u_ptr<buffer> index_buffer_;
    uint32_t index_count_;

    std::vector<skinning_utils::mesh_group>  mesh_groups_;
    std::vector<skinning_utils::material>    materials_;
    std::vector<s_ptr<skinning_utils::node>> nodes_;
    std::vector<int> root_nodes_;

    bool has_skin_ = false;
    skinning_utils::skin_info skin_info_;
    node_info node_info_;

    std::vector<skinning_utils::image_info>   images_;
    std::vector<skinning_utils::texture_info> textures_;

    // for node_info desc buffer
    u_ptr<descriptor_pool>       desc_pool_;
    u_ptr<buffer>                desc_buffer_;
    VkDescriptorSet              desc_set_;

    static u_ptr<descriptor_set_layout> desc_set_layout_;
};

}} // hnll::graphics