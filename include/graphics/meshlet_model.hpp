#pragma once

// std
#include <vector>
#include <memory>
#include <stdint.h>

// lib
#include <eigen3/Eigen/Dense>
#include <vulkan/vulkan.h>

namespace hnll {

using vec3 = Eigen::Vector3f;

namespace graphics {

// forward declaration
class device;
class buffer;
class descriptor_pool;
class descriptor_set_layout;
struct frame_info;
struct vertex;
struct mesh_builder;
template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

constexpr uint32_t VERTEX_DESC_ID  = 0;
constexpr uint32_t MESHLET_DESC_ID = 1;
constexpr uint32_t DESC_SET_COUNT  = 2;
constexpr uint32_t MAX_VERTEX_PER_MESHLET = 64;
constexpr uint32_t MAX_INDEX_PER_MESHLET  = 378;

struct meshlet
{
  uint32_t vertex_indices   [MAX_VERTEX_PER_MESHLET]; // indicates position in a vertex buffer
  uint32_t primitive_indices[MAX_INDEX_PER_MESHLET];
  uint32_t vertex_count;
  uint32_t index_count;
  // for frustum culling (for bounding sphere)
  alignas(16) vec3 center;
  float            radius;
  // for aabb
  // alignas(16) vec3 radius;
};

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
      VkCommandBuffer  _command_buffer,
      VkDescriptorSet  _global_desc_set,
      VkPipelineLayout _pipeline_layout);
    void draw(VkCommandBuffer  _command_buffer);

    // getter
    const buffer& get_vertex_buffer()  const;
    const buffer& get_meshlet_buffer() const;
    inline void* get_raw_vertices_data() { return raw_vertices_.data(); }
    inline void* get_meshlets_data()     { return meshlets_.data(); }
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