#pragma once

// std
#include <vector>
#include <memory>
#include <stdint.h>

// lib
#include <eigen3/Eigen/Dense>

namespace hnll {

using vec3 = Eigen::Vector3f;

namespace graphics {

// forward declaration
class device;
class buffer;
template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

template<uint32_t MAX_VERTEX_PER_MESHLET = 64, uint32_t MAX_INDICES_PER_MESHLET = 378>
struct meshlet
{
  uint32_t vertex_indices   [MAX_VERTEX_PER_MESHLET]; // indicates position in a vertex buffer
  uint32_t primitive_indices[MAX_INDICES_PER_MESHLET];
  uint32_t vertex_count;
  uint32_t index_count;
};

class meshlet_model
{
  public:
    struct vertex
    {
      alignas(16) vec3 position;
      alignas(16) vec3 normal;
      alignas(16) vec3 color;
    };

    meshlet_model(std::vector<vertex>&& raw_vertices, std::vector<meshlet<>>&& meshlets);

    static u_ptr<meshlet_model> create(
      device& _device,
      std::vector<vertex>&& raw_vertices,
      std::vector<meshlet<>>&& meshlets
    );

    const buffer& get_vertex_buffer()  const;
    const buffer& get_meshlet_buffer() const;
    inline void* get_raw_vertices_data() { return raw_vertices_.data(); }
    inline void* get_meshlets_data()     { return meshlets_.data(); }

  private:
    void create_vertex_buffer(device& _device);
    void create_meshlet_buffer(device& _device);

    std::vector<vertex>    raw_vertices_;
    std::vector<meshlet<>> meshlets_;
    u_ptr<buffer> vertex_buffer_;
    u_ptr<buffer> meshlet_buffer_;
};

}} // namespace hnll::graphics