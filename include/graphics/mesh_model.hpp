#pragma once

// hnll
#include <graphics/device.hpp>
#include <graphics/buffer.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <unordered_map>
#include <memory>
#include <string>

#pragma once

#include <functional>

namespace hnll {
namespace graphics {

// https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/57595105#57595105
template <typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
{
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hash_combine(seed, rest), ...);
}

class mesh_model
{
  public:
    using map = std::unordered_map<std::string, std::shared_ptr<mesh_model>>;
    // compatible with wavefront obj. file
    struct vertex
    {
      glm::vec3 position{};
      glm::vec3 color{};
      glm::vec3 normal{};
      // texture cordinates
      glm::vec2 uv{};
      // return a description compatible with the shader
      static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
      static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();

      bool operator==(const vertex& other) const
      { return position == other.position && color == other.color && normal == other.normal && uv == other.uv; }
    };

    struct builder
    {
      // copied to the vertex buffer and index buffer
      std::vector<vertex> vertices{};
      std::vector<uint32_t> indices{};

      void load_model(const std::string& filename);
    };

    mesh_model(device& device, const mesh_model::builder &builder);
    ~mesh_model();

    mesh_model(const mesh_model &) = delete;
    mesh_model& operator=(const mesh_model &) = delete;

    static std::shared_ptr<mesh_model> create_model_from_file(device &device, const std::string &filename);

    void bind(VkCommandBuffer command_buffer);
    void draw(VkCommandBuffer command_buffer);

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
};

} // namespace graphics
} // namespace hnll
