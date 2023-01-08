// hnll
#include <graphics/frame_anim_meshlet_model.hpp>
#include <graphics/buffer.hpp>

namespace hnll::graphics {

u_ptr<frame_anim_meshlet_model> frame_anim_meshlet_model::create_from_skinning_mesh_model(
  hnll::graphics::device &_device,
  hnll::graphics::skinning_mesh_model &original,
  uint32_t max_fps)
{

}

frame_anim_meshlet_model::frame_anim_meshlet_model(device &_device) : device_(_device)
{}

} // namespace hnll::graphics