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
class descriptor_set;
class skinning_mesh_model;
namespace skinning_utils {
  struct animation;
  struct builder;
}

class frame_anim_meshlet_model
{
  public:
    static u_ptr<frame_anim_meshlet_model> create_from_skinning_mesh_model(skinning_mesh_model& original, uint32_t max_fps = frame_anim_utils::MAX_FPS);
    explicit frame_anim_meshlet_model(device& _device);

    void bind(
      uint32_t animation_index,
      uint32_t frame_index,
      VkCommandBuffer command_buffer,
      const std::vector<VkDescriptorSet>& external_desc_sets,
      VkPipelineLayout pipeline_layout);
    void draw(VkCommandBuffer command_buffer);

    // getter
    uint32_t get_animation_count() const { return dynamic_attributes_buffers_.size(); }
    uint32_t get_frame_count(uint32_t animation_index) const { return frame_counts_[animation_index]; }
    float    get_start_time(uint32_t animation_index)  const { return start_times_[animation_index]; }
    float    get_end_time(uint32_t animation_index)    const { return end_times_[animation_index]; }
    const std::vector<frame_anim_utils::dynamic_attributes>& get_initial_dynamic_attribs() const { return initial_dynamic_attribs_; }
    const std::vector<uint32_t>& get_raw_indices() const { return raw_indices_; }

    static std::vector<u_ptr<descriptor_set_layout>> default_desc_set_layouts(device& _device);

    // setter
    void set_meshlets(std::vector<animated_meshlet_pack::meshlet>&& meshlets)
    { meshlets_ = std::move(meshlets); meshlet_count_ = meshlets_.size(); }
    // temp
    void set_initial_sphere(std::vector<vec4>&& spheres) { initial_spheres_ = std::move(spheres); }
  private:
    void load_from_skinning_mesh_model(skinning_mesh_model& original, uint32_t max_fps);
    void create_meshlets_buffer();
    void setup_descs();

    device& device_;
    // index
    u_ptr<buffer> index_buffer_;

    // desc buffers for static objects
    // uv, color, joint info
    u_ptr<buffer> common_attributes_buffer_;
    // this meshlet info is used by all frames
    u_ptr<buffer> meshlet_buffer_;

    // desc buffers for dynamic objects
    // first index : animation index
    // second index : animation frame
    std::vector<std::vector<u_ptr<buffer>>> dynamic_attributes_buffers_;
    std::vector<std::vector<u_ptr<buffer>>> sphere_buffers_;

    std::vector<uint32_t> frame_counts_;
    std::vector<uint32_t> accumulative_frame_counts_;
    uint32_t vertex_count_;
    uint32_t index_count_;
    uint32_t meshlet_count_;

    // desc sets
    u_ptr<descriptor_set> common_desc_sets_;
    u_ptr<descriptor_set> dynamic_attribs_desc_sets_;
    u_ptr<descriptor_set> sphere_desc_sets_;

    std::vector<float> start_times_;
    std::vector<float> end_times_;

    // raw data for (and from) mesh separation (temporary)
    std::vector<frame_anim_utils::dynamic_attributes> initial_dynamic_attribs_;
    std::vector<uint32_t> raw_indices_;
    std::vector<animated_meshlet_pack::meshlet> meshlets_;
    // temp
    std::vector<vec4> initial_spheres_;
};

}} // namespace hnll::graphics