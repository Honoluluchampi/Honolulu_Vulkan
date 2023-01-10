// hnll
#include <graphics/frame_anim_meshlet_model.hpp>
#include <graphics/skinning_mesh_model.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <graphics/descriptor_set.hpp>
#include <graphics/buffer.hpp>
#include <geometry/mesh_separation.hpp>
#include <geometry/mesh_model.hpp>

//std
#include <iostream>

namespace hnll::graphics {

#define FRAME_ANIM_DESC_SET_COUNT 4

u_ptr<frame_anim_meshlet_model> frame_anim_meshlet_model::create_from_skinning_mesh_model(
  hnll::graphics::skinning_mesh_model &original,
  uint32_t max_fps)
{
  auto ret = std::make_unique<frame_anim_meshlet_model>(original.get_device());
  ret->load_from_skinning_mesh_model(original, max_fps);

  auto geometry_model = geometry::mesh_model::create_from_dynamic_attributes(
    ret->get_ownership_of_raw_dynamic_attribs(),
    ret->get_raw_indices()
  );
  auto meshlet_pack = geometry::mesh_separation::separate_into_meshlet_pack(geometry_model);
  ret->set_meshlets(std::move(meshlet_pack.meshlets));
  ret->set_raw_spheres(std::move(meshlet_pack.spheres));
  ret->setup_descs();

  return ret;
}

frame_anim_meshlet_model::frame_anim_meshlet_model(device &_device) : device_(_device)
{}

void frame_anim_meshlet_model::setup_descs()
{
  // setup buffers for desc sets
  create_buffers();

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
    desc_set_count,
    desc_set_count,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

  dynamic_attribs_desc_sets_->add_layout(static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_TASK_BIT_NV | VK_SHADER_STAGE_MESH_BIT_NV));

  for (int i = 0; i < dynamic_attributes_buffers_.size(); i++)
    for (int j = 0; j < dynamic_attributes_buffers_[i].size(); j++)
      dynamic_attribs_desc_sets_->add_buffer(std::move(dynamic_attributes_buffers_[i][j]));

  dynamic_attribs_desc_sets_->build_sets();

  // sphere ------------------------------------------
  sphere_desc_sets_ = descriptor_set::create(device_);

  sphere_desc_sets_->create_pool(
    desc_set_count,
    desc_set_count,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

  sphere_desc_sets_->add_layout(static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_TASK_BIT_NV | VK_SHADER_STAGE_MESH_BIT_NV));

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
  // temp
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

void frame_anim_meshlet_model::load_from_skinning_mesh_model(skinning_mesh_model &original, uint32_t max_fps)
{
  auto builder = original.get_ownership_of_builder();

  vertex_count_ = builder.vertex_buffer.size();
  index_count_ = builder.index_buffer.size();

  // extract time info
  auto& animations = original.get_animations();
  start_times_.resize(animations.size());
  end_times_.resize(animations.size());
  for (int i = 0; i < animations.size(); i++) {
    start_times_[i] = animations[i].start;
    end_times_[i]   = animations[i].end;
  }

  // extract common attributes
  std::vector<frame_anim_utils::common_attributes> common_attribs;
  for (auto& data : builder.vertex_buffer) {
    frame_anim_utils::common_attributes new_ca;
    new_ca.uv0     = data.tex_coord_0;
    new_ca.uv1     = data.tex_coord_1;
    new_ca.color   = data.color;
    common_attribs.emplace_back(std::move(new_ca));
  }
  common_attributes_buffer_ = buffer::create_with_staging(
    device_,
    common_attribs.size() * sizeof(frame_anim_utils::common_attributes),
    1,
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    common_attribs.data()
  );

  // extract index buffer
  index_buffer_ = buffer::create_with_staging(
    device_,
    builder.index_buffer.size() * sizeof(uint32_t),
    1,
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    builder.index_buffer.data()
  );

  // for mesh separation
  raw_indices_ = builder.index_buffer;

  // extract dynamic attributes
  auto animation_count = original.get_animations().size();
  dynamic_attributes_buffers_.resize(animation_count);
  for (int i = 0; i < animation_count; i++) {
    auto& anim = original.get_animations()[i];
    float timer = anim.start;
    while (timer <= anim.end) {
      // calculate new position and normal
      original.update_animation(i, timer);
      auto new_dynamic_attribs = frame_anim_utils::extract_dynamic_attributes(original, builder);

      // assign buffer
      auto new_buffer = buffer::create_with_staging(
        device_,
        vertex_count_ * sizeof(frame_anim_utils::dynamic_attributes),
        1,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        new_dynamic_attribs.data()
      );
      dynamic_attributes_buffers_[i].emplace_back(std::move(new_buffer));

      // for mesh separation
      raw_dynamic_attribs_.emplace_back(std::move(new_dynamic_attribs));

      // update timer
      timer += 1.f / static_cast<float>(max_fps);
    }
  }

  frame_counts_.resize(animation_count);
  accumulative_frame_counts_.resize(animation_count);
  accumulative_frame_counts_[0] = 0;

  for (int i = 0; i < dynamic_attributes_buffers_.size(); i++) {
    uint32_t count = dynamic_attributes_buffers_[i].size();
    frame_counts_[i] = count;
    if (i != 0)
      accumulative_frame_counts_[i] = accumulative_frame_counts_[i - 1] + frame_counts_[i - 1];
  }
}

void frame_anim_meshlet_model::create_buffers()
{
  meshlet_buffer_ = buffer::create_with_staging(
    device_,
    meshlets_.size() * sizeof(animated_meshlet_pack::meshlet),
    1,
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    meshlets_.data()
  );

  sphere_buffers_.resize(1);
  sphere_buffers_[0].resize(frame_counts_[0]);
  for (int i = 0; i < frame_counts_[0]; i++) {
    sphere_buffers_[0][i] = buffer::create_with_staging(
      device_,
      raw_spheres_[i].size() * sizeof(vec4),
      1,
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      raw_spheres_[i].data()
    );
  }
}

} // namespace hnll::graphics