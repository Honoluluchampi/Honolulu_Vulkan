// hnll
#include <graphics/frame_anim_meshlet_model.hpp>
#include <graphics/descriptor_set_layout.hpp>
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

} // namespace hnll::graphics