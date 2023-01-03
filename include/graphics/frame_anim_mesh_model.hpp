#pragma once

// hnll
#include <graphics/buffer.hpp>
#include <utils/common_using.hpp>

namespace hnll {

namespace graphics {

// forward declaration
class skinning_mesh_model;
namespace skinning_utils { struct animation; }

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

    static u_ptr<frame_anim_mesh_model> create_from_skinning_mesh_model(device& _device, skinning_mesh_model& original, uint32_t max_fps);
    frame_anim_mesh_model(device& _device);

  private:
    void load_from_skinning_mesh_model(skinning_mesh_model& original, uint32_t max_fps);
    void load_animation(
      skinning_mesh_model& original,
      uint32_t animation_index,
      uint32_t max_fps);

    device& device_;
    // uv, color, joint info
    u_ptr<buffer> common_attributes_buffer_;
    // index
    u_ptr<buffer> index_buffer_;
    // first vector : animation index
    // second vector : animation frame
    std::vector<std::vector<u_ptr<buffer>>> dynamic_attributes_buffers_;
};

}} // namespace hnll::graphics