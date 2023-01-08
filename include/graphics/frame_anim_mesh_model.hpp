#pragma once

// hnll
#include <utils/common_using.hpp>
#include <graphics/frame_anim_utils.hpp>

namespace hnll {

namespace graphics {

// forward declaration
class device;
class buffer;
class skinning_mesh_model;
namespace skinning_utils {
  struct animation;
  struct builder;
}

class frame_anim_mesh_model
{
  public:

    static u_ptr<frame_anim_mesh_model> create_from_skinning_mesh_model(skinning_mesh_model& original, uint32_t max_fps = frame_anim_utils::MAX_FPS);
    frame_anim_mesh_model(device& _device);

    void bind(uint32_t animation_index, uint32_t frame_index, VkCommandBuffer command_buffer);
    void draw(VkCommandBuffer command_buffer);

    // getter
    uint32_t get_animation_count() const { return dynamic_attributes_buffers_.size(); }
    uint32_t get_frame_count(uint32_t animation_index) const { return frame_counts_[animation_index]; }
    float    get_start_time(uint32_t animation_index)  const { return start_times_[animation_index]; }
    float    get_end_time(uint32_t animation_index)    const { return end_times_[animation_index]; }

    // vertex attributes binding info
    static std::vector<VkVertexInputBindingDescription>   get_binding_descriptions();
    static std::vector<VkVertexInputAttributeDescription> get_attributes_descriptions();

  private:
    void load_from_skinning_mesh_model(skinning_mesh_model& original, uint32_t max_fps);

    device& device_;
    // uv, color, joint info
    u_ptr<buffer> common_attributes_buffer_;
    // index
    u_ptr<buffer> index_buffer_;
    // first vector : animation index
    // second vector : animation frame
    std::vector<std::vector<u_ptr<buffer>>> dynamic_attributes_buffers_;

    std::vector<uint32_t> frame_counts_;
    uint32_t vertex_count_;
    uint32_t index_count_;

    std::vector<float> start_times_;
    std::vector<float> end_times_;
};

}} // namespace hnll::graphics