// hnll
#include <graphics/frame_anim_mesh_model.hpp>
#include <graphics/skinning_mesh_model.hpp>

namespace hnll::graphics {

#define MAX_FPS 60

frame_anim_mesh_model::frame_anim_mesh_model(device &_device) : device_(_device) {}

u_ptr<frame_anim_mesh_model> frame_anim_mesh_model::create_from_skinning_mesh_model(device& _device, skinning_mesh_model &original, uint32_t max_fps = MAX_FPS)
{
  auto ret = std::make_unique<frame_anim_mesh_model>(_device);
  ret->load_from_skinning_mesh_model(original, max_fps);

  return ret;
}

void frame_anim_mesh_model::load_from_skinning_mesh_model(hnll::graphics::skinning_mesh_model &original, uint32_t max_fps)
{
  auto original_data = original.get_ownership_of_builder();

  // extract common attributes
  std::vector<common_attributes> common_attribs;
  for (auto& data : original_data.vertex_buffer) {
    common_attributes new_ca;
    new_ca.uv0     = data.tex_coord_0;
    new_ca.uv1     = data.tex_coord_1;
    new_ca.color   = data.color;
    new_ca.joints  = data.joint_indices;
    new_ca.weights = data.joint_weights;
    common_attribs.emplace_back(std::move(new_ca));
  }
  common_attributes_buffer_ = buffer::create_with_staging(
    device_,
    common_attribs.size() * sizeof(common_attributes),
    1,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    common_attribs.data()
  );

  // extract dynamic attributes
  auto animation_count = original.get_animations().size();
  dynamic_attributes_buffers_.resize(animation_count);
  for (int i = 0; i < animation_count; i++) {
    load_animation(original, i, max_fps);
  }
}

void frame_anim_mesh_model::load_animation(skinning_mesh_model& original, uint32_t animation_index, uint32_t max_fps)
{
  auto& anim = original.get_animations()[animation_index];
  float timer = anim.start;
  while (timer > anim.end) {
    // calculate new position and normal
    std::vector<dynamic_attributes> new_dynamic_attribs;

    // assign buffer
    auto new_buffer = buffer::create_with_staging(
      device_,
      new_dynamic_attribs.size() * sizeof(dynamic_attributes),
      1,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      new_dynamic_attribs.data()
    );
    dynamic_attributes_buffers_[animation_index].emplace_back(std::move(new_buffer));

    // update timer
    timer += 1.f / static_cast<float>(max_fps);
  }
}

} // namespace hnll::graphics