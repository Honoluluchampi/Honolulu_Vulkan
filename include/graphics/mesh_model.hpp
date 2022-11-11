#pragma once

// hnll
#include <graphics/device.hpp>
#include <graphics/buffer.hpp>
#include <graphics/utils.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <eigen3/Eigen/Dense>

// std
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>

// forward declaration
namespace hnll::geometry { class mesh_model; }

namespace hnll::graphics {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

class mesh_model
{
  public:
    // compatible with wavefront obj. file

    mesh_model(device& device, const mesh_builder &builder);
    ~mesh_model();

    mesh_model(const mesh_model &) = delete;
    mesh_model& operator=(const mesh_model &) = delete;

    static s_ptr<mesh_model> create_model_from_file(device &device, const std::string &filename);
    static s_ptr<mesh_model> create_from_geometry_mesh_model(device &device, const s_ptr<geometry::mesh_model>& gm);

    void bind(VkCommandBuffer command_buffer);
    void draw(VkCommandBuffer command_buffer);

    // setter
    bool has_index_buffer() const { return had_index_buffer_; }
    // getter
    const std::vector<vertex>&   get_vertex_list() const { return vertex_list_; }
    std::vector<Eigen::Vector3d> get_vertex_position_list() const;
    unsigned                     get_face_count() const { return index_count_ / 3; }
  private:
    void create_vertex_buffers(const std::vector<vertex> &vertices);
    void create_index_buffers(const std::vector<uint32_t> &indices);

    device& device_;
    // contains buffer itself and buffer memory
    std::unique_ptr<buffer> vertex_buffer_;
    std::unique_ptr<buffer> index_buffer_;
    uint32_t vertex_count_;
    uint32_t index_count_;

    bool had_index_buffer_ = false;

    // for geometric process
    std::vector<vertex> vertex_list_{};
};

} // namespace hnll::graphics