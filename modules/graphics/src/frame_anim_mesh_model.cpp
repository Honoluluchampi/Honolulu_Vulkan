// hnll
#include <graphics/buffer.hpp>
#include <graphics/frame_anim_mesh_model.hpp>
#include <graphics/skinning_mesh_model.hpp>

namespace hnll::graphics {

#define MAX_FPS 60

frame_anim_mesh_model::frame_anim_mesh_model(device &_device) : device_(_device) {}

u_ptr<frame_anim_mesh_model> frame_anim_mesh_model::create_from_skinning_mesh_model(device& _device, skinning_mesh_model &original, uint32_t max_fps)
{
  auto ret = std::make_unique<frame_anim_mesh_model>(_device);
  ret->load_from_skinning_mesh_model(original, max_fps);

  return ret;
}

void frame_anim_mesh_model::load_from_skinning_mesh_model(hnll::graphics::skinning_mesh_model &original, uint32_t max_fps)
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
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
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
        144 * sizeof(frame_anim_utils::dynamic_attributes),
        1,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        new_dynamic_attribs.data()
      );
      dynamic_attributes_buffers_[i].emplace_back(std::move(new_buffer));

      // update timer
      timer += 1.f / static_cast<float>(max_fps);
    }
  }

  frame_counts_.resize(dynamic_attributes_buffers_.size());
  for (int i = 0; i < dynamic_attributes_buffers_.size(); i++) {
    frame_counts_[i] = dynamic_attributes_buffers_[i].size();
  }
}

void frame_anim_mesh_model::bind(uint32_t animation_index, uint32_t frame_index, VkCommandBuffer command_buffer)
{
  // bind vertex buffer
  VkBuffer vertex_buffers[] = {
    dynamic_attributes_buffers_[animation_index][frame_index]->get_buffer(),
    common_attributes_buffer_->get_buffer()
  };
  VkDeviceSize offsets[] = { 0, 0 };
  vkCmdBindVertexBuffers(command_buffer, 0, 2, vertex_buffers, offsets);

  vkCmdBindIndexBuffer(command_buffer, index_buffer_->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
}

void frame_anim_mesh_model::draw(VkCommandBuffer command_buffer)
{
  vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
}

std::vector<VkVertexInputBindingDescription>   frame_anim_mesh_model::get_binding_descriptions()
{
  std::vector<VkVertexInputBindingDescription> binding_descriptions;
  binding_descriptions.resize(2);
  // binding for dynamic attribs
  binding_descriptions[0].binding   = 0;
  binding_descriptions[0].stride    = sizeof(frame_anim_utils::dynamic_attributes);
  binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  // binding for common attribs
  binding_descriptions[1].binding   = 1;
  binding_descriptions[1].stride    = sizeof(frame_anim_utils::common_attributes);
  binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> frame_anim_mesh_model::get_attributes_descriptions()
{
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
  attribute_descriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(frame_anim_utils::dynamic_attributes, position)});
  attribute_descriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(frame_anim_utils::dynamic_attributes, normal)});
  attribute_descriptions.push_back({2, 1, VK_FORMAT_R32G32_SFLOAT,       offsetof(frame_anim_utils::common_attributes, uv0)});
  attribute_descriptions.push_back({3, 1, VK_FORMAT_R32G32_SFLOAT,       offsetof(frame_anim_utils::common_attributes, uv1)});
  attribute_descriptions.push_back({4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(frame_anim_utils::common_attributes, color)});

  return attribute_descriptions;
}

} // namespace hnll::graphics