// hnll
#include <graphics/frame_anim_meshlet_model.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <graphics/descriptor_set.hpp>
#include <graphics/buffer.hpp>

namespace hnll::graphics {

#define FRAME_ANIM_DESC_SET_COUNT 4

u_ptr<frame_anim_meshlet_model> frame_anim_meshlet_model::create_from_skinning_mesh_model(
  hnll::graphics::device &_device,
  hnll::graphics::skinning_mesh_model &original,
  uint32_t max_fps)
{

}

frame_anim_meshlet_model::frame_anim_meshlet_model(device &_device) : device_(_device)
{}

void frame_anim_meshlet_model::setup_descs()
{
  // common -----------------------------------------
  common_desc_sets_ = descriptor_set::create(device_);

  common_desc_sets_->create_pool(
    FRAME_ANIM_DESC_SET_COUNT + 1,
    2,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

  common_desc_sets_->add_layout(static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_TASK_BIT_NV | VK_SHADER_STAGE_MESH_BIT_NV));

  common_desc_sets_->add_buffer(std::move(common_attributes_buffer_));
  common_desc_sets_->add_buffer(std::move(meshlet_buffer_));

  common_desc_sets_->build_sets();

  // dynamic attribs ----------------------------------
  uint32_t desc_set_count = 0;
  for (auto& count : frame_counts_) {
    desc_set_count += count;
  }

  dynamic_attribs_desc_sets_ = descriptor_set::create(device_);

  dynamic_attribs_desc_sets_->create_pool(
    FRAME_ANIM_DESC_SET_COUNT + 1,
    desc_set_count,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

  for (int i = 0; i < dynamic_attributes_buffers_.size(); i++)
    for (int j = 0; j < dynamic_attributes_buffers_[i].size(); j++)
      dynamic_attribs_desc_sets_->add_buffer(std::move(dynamic_attributes_buffers_[i][j]));

  dynamic_attribs_desc_sets_->build_sets();

  // sphere ------------------------------------------
  sphere_desc_sets_ = descriptor_set::create(device_);

  sphere_desc_sets_->create_pool(
    FRAME_ANIM_DESC_SET_COUNT + 1,
    desc_set_count,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

  for (int i = 0; i < sphere_buffers_.size(); i++)
    for (int j = 0; j < sphere_buffers_[i].size(); j++)
      sphere_desc_sets_->add_buffer(std::move(sphere_buffers_[i][j]));

  sphere_desc_sets_->build_sets();
}

std::vector<u_ptr<descriptor_set_layout>> frame_anim_meshlet_model::default_desc_set_layouts(device& _device)
{
  std::vector<u_ptr<descriptor_set_layout>> ret;

  ret.resize(FRAME_ANIM_DESC_SET_COUNT);

  for (int i = 0; i < FRAME_ANIM_DESC_SET_COUNT; i++) {
    ret[i] = descriptor_set_layout::builder(_device)
      .add_binding(
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_SHADER_STAGE_TASK_BIT_NV | VK_SHADER_STAGE_MESH_BIT_NV)
      .build();
  }

  return ret;
}

void frame_anim_meshlet_model::bind(
  uint32_t animation_index,
  uint32_t frame_index,
  VkCommandBuffer command_buffer,
  const std::vector<VkDescriptorSet> &external_desc_sets,
  VkPipelineLayout pipeline_layout)
{
  std::vector<VkDescriptorSet> desc_sets;
  for (const auto& set : external_desc_sets) {
    desc_sets.push_back(set);
  }
  desc_sets.push_back(common_desc_sets_->get_set(0)); // common attribs
  desc_sets.push_back(common_desc_sets_->get_set(1)); // meshlet
  uint32_t anim_frame_index = accumulative_frame_counts_[animation_index] + frame_index;
  desc_sets.push_back(dynamic_attribs_desc_sets_->get_set(anim_frame_index));
  desc_sets.push_back(sphere_desc_sets_->get_set(anim_frame_index));

  vkCmdBindDescriptorSets(
    command_buffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline_layout,
    0,
    static_cast<uint32_t>(desc_sets.size()),
    desc_sets.data(),
    0,
    nullptr
  );
}

void frame_anim_meshlet_model::draw(VkCommandBuffer command_buffer)
{
  vkCmdDrawMeshTasksNV(
    command_buffer,
    meshlet_count_,
    0
  );
}
} // namespace hnll::graphics