// hnll
#include <game/actors/virtual_camera.hpp>
#include <game/components/keyboard_movement_component.hpp>
#include <game/components/wire_frame_frustum_component.hpp>
#include <game/components/viewer_component.hpp>
#include <game/modules/graphics_engine.hpp>
#include <utils/rendering_utils.hpp>

namespace hnll::game {

s_ptr<virtual_camera> virtual_camera::create(graphics_engine& _engine)
{
  auto& device = _engine.get_device_r();
  auto camera = std::make_shared<virtual_camera>();
  camera->viewer_comp_ = game::viewer_component::create(*camera->get_transform_sp(), _engine.get_renderer_r());
  camera->viewer_comp_->auto_update_view_frustum();
  auto frustum = geometry::perspective_frustum::create(M_PI / 4.f, M_PI / 4.f, 1.f, 15.f);
  camera->wire_frustum_comp_ = game::wire_frame_frustum_component::create(camera, frustum, device);
  camera->key_comp_ = std::make_shared<game::keyboard_movement_component>(_engine.get_window_r().get_glfw_window(), *camera->get_transform_sp());
  camera->add_component(camera->key_comp_);
  camera->key_comp_->set_updating_off();
  return camera;
}

utils::frustum_info virtual_camera::get_frustum_info()
{
  auto& frustum = get_perspective_frustum();
  const auto& transform = get_transform_sp();
  return {
    { transform->translation.x, transform->translation.y, transform->translation.z},
    frustum.get_near_p().cast<float>(),
    frustum.get_far_p().cast<float>(),
    frustum.get_top_n().cast<float>(),
    frustum.get_bottom_n().cast<float>(),
    frustum.get_right_n().cast<float>(),
    frustum.get_left_n().cast<float>(),
    frustum.get_near_n().cast<float>(),
    frustum.get_far_n().cast<float>(),
  };
}

void virtual_camera::update_frustum_planes()
{
  wire_frustum_comp_->update_frustum_planes(*get_transform_sp());
}

// getter
const geometry::perspective_frustum& virtual_camera::get_perspective_frustum() const
{ return wire_frustum_comp_->get_perspective_frustum(); }

// setter
void virtual_camera::set_movement_updating_on()  { key_comp_->set_updating_on(); }
void virtual_camera::set_movement_updating_off() { key_comp_->set_updating_off(); }

} // namespace hnll::game