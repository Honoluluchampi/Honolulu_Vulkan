#pragma once

// hnll
#include <graphics/buffer.hpp>
#include <utils/common_using.hpp>

namespace hnll {

namespace graphics {

// forward declaration
class skinning_mesh_model;
namespace skinning_utils {
  struct animation;
  struct builder;
}

#define MAX_FPS 60

class frame_anim_mesh_model
{
  public:
    struct dynamic_attributes
    {
      vec3 position;
      vec3 normal;
    };
    struct common_attributes
    {
      vec2 uv0;
      vec2 uv1;
      vec4 color;
      vec4 joints;
      vec4 weights;
    };

    static u_ptr<frame_anim_mesh_model> create_from_skinning_mesh_model(device& _device, skinning_mesh_model& original, uint32_t max_fps = MAX_FPS);
    frame_anim_mesh_model(device& _device);

    void bind(uint32_t animation_index, uint32_t frame_index, VkCommandBuffer command_buffer);
    void draw(VkCommandBuffer command_buffer);

    // getter
    uint32_t get_max_frame_index(uint32_t animation_index) { return max_frame_indices_[animation_index]; }

    // vertex attributes binding info
    static std::vector<VkVertexInputBindingDescription>   get_binding_descriptions();
    static std::vector<VkVertexInputAttributeDescription> get_attributes_descriptions();

  private:
    void load_from_skinning_mesh_model(skinning_mesh_model& original, uint32_t max_fps);
    std::vector<dynamic_attributes> extract_dynamic_attributes(
      skinning_mesh_model& original,
      const skinning_utils::builder& builder);

    device& device_;
    // uv, color, joint info
    u_ptr<buffer> common_attributes_buffer_;
    // index
    u_ptr<buffer> index_buffer_;
    // first vector : animation index
    // second vector : animation frame
    std::vector<std::vector<u_ptr<buffer>>> dynamic_attributes_buffers_;

    std::vector<uint32_t> max_frame_indices_;
    uint32_t vertex_count_;
    uint32_t index_count_;
};

}} // namespace hnll::graphics