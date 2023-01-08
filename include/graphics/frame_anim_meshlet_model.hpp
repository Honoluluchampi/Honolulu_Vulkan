#pragma once

// hnll
#include <utils/common_using.hpp>
#include <graphics/frame_anim_utils.hpp>
#include <graphics/meshlet_utils.hpp>

//lib
#include <vulkan/vulkan.h>

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

class frame_anim_meshlet_model
{
  public:
    static u_ptr<frame_anim_meshlet_model> create_from_skinning_mesh_model(device& _device, skinning_mesh_model& original, uint32_t max_fps = frame_anim_utils::MAX_FPS);
    frame_anim_meshlet_model(device& _device);

    void bind(
      uint32_t animation_index,
      uint32_t frame_index,
      VkCommandBuffer command_buffer,
      const std::vector<VkDescriptorSet>& external_desc_sets,
      VkPipelineLayout pipeline_layout){}
    void draw(VkCommandBuffer command_buffer){}

    // getter
    uint32_t get_animation_count() const { return dynamic_attributes_buffers_.size(); }
    uint32_t get_frame_count(uint32_t animation_index) const { return frame_counts_[animation_index]; }
    float    get_start_time(uint32_t animation_index)  const { return start_times_[animation_index]; }
    float    get_end_time(uint32_t animation_index)    const { return end_times_[animation_index]; }

    static std::vector<u_ptr<descriptor_set_layout>> default_desc_set_layouts(device& _device);

  private:
    void load_from_skinning_mesh_model(skinning_mesh_model& original, uint32_t max_fps);

    device& device_;
    // uv, color, joint info
    u_ptr<buffer> common_attributes_buffer_;
    // index
    u_ptr<buffer> index_buffer_;
    // first index : animation index
    // second index : animation frame
    std::vector<std::vector<u_ptr<buffer>>> dynamic_attributes_buffers_;

    // this meshlet info is used by all frames
    u_ptr<buffer> meshlet_buffer_;
    // bounding sphere is assigned for each frame's each meshlet
    // index usage is same as dynamic_attributes_buffers_
    std::vector<std::vector<u_ptr<buffer>>> sphere_buffers_;

    std::vector<uint32_t> frame_counts_;
    uint32_t vertex_count_;
    uint32_t index_count_;

    std::vector<float> start_times_;
    std::vector<float> end_times_;
};

}} // namespace hnll::graphics