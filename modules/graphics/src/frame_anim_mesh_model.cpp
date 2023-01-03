// hnll
#include <graphics/frame_anim_mesh_model.hpp>
#include <graphics/skinning_mesh_model.hpp>

namespace hnll::graphics {

#define MAX_FPS 60

u_ptr<frame_anim_mesh_model> frame_anim_mesh_model::create_from_skinning_mesh_model(device& _device, skinning_mesh_model &original, uint32_t max_fps = MAX_FPS)
{
  auto ret = std::make_unique<frame_anim_mesh_model>(_device);
  ret->load_from_skinning_mesh_model(original);

  return ret;
}

void frame_anim_mesh_model::load_from_skinning_mesh_model(hnll::graphics::skinning_mesh_model &original)
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
}

frame_anim_mesh_model::frame_anim_mesh_model(device &_device) : device_(_device) {}
} // namespcae hnll::graphics