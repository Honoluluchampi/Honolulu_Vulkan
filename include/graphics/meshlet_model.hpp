#pragma once
// hnll
#include <utils/common_using.hpp>
#include <graphics/meshlet_utils.hpp>

// std
#include <vector>
#include <memory>
#include <stdint.h>

// lib
#include <eigen3/Eigen/Dense>
#include <vulkan/vulkan.h>

namespace hnll {
namespace graphics {

// forward declaration
class device;
class buffer;
class descriptor_pool;
class descriptor_set_layout;
struct frame_info;
struct vertex;
struct mesh_builder;

class meshlet_model
{
  public:

    meshlet_model(std::vector<vertex>&& raw_vertices, std::vector<meshlet>&& meshlets);

    static u_ptr<meshlet_model> create(
      device& _device,
      std::vector<vertex>&&  _raw_vertices,
      std::vector<meshlet>&& _meshlets
    );

    static u_ptr<meshlet_model> create_from_file(device& _device, std::string _filename);

    void bind(
      VkCommandBuffer               _command_buffer,
      std::vector<VkDescriptorSet>  _external_desc_set,
      VkPipelineLayout              _pipeline_layout);
    void draw(VkCommandBuffer  _command_buffer);

    // getter
    const buffer& get_vertex_buffer()  const;
    const buffer& get_meshlet_buffer() const;
    inline void* get_raw_vertices_data() { return raw_vertices_.data(); }
    inline void* get_meshlets_data()     { return meshlets_.data(); }
    inline uint32_t get_meshlets_count() { return meshlets_.size(); }
    std::vector<VkDescriptorSetLayout> get_raw_desc_set_layouts() const;

    static std::vector<u_ptr<descriptor_set_layout>> default_desc_set_layouts(device& _device);

  private:
    void setup_descs(device& _device);
    void create_desc_pool(device& _device);
    void create_desc_buffers(device& _device);
    void create_desc_set_layouts(device& _device);
    void create_desc_sets();

    std::vector<vertex>  raw_vertices_;
    std::vector<meshlet> meshlets_;
    u_ptr<descriptor_pool>                    desc_pool_;
    std::vector<u_ptr<buffer>>                desc_buffers_;
    std::vector<u_ptr<descriptor_set_layout>> desc_set_layouts_;
    std::vector<VkDescriptorSet>              desc_sets_;
};

}} // namespace hnll::graphics