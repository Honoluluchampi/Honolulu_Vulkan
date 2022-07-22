// iscg
#include <control_point.hpp>

namespace iscg {

control_point::control_point(const glm::vec3 &position, const glm::vec3 &color, float radius)
  : hnll::game::actor()
{
  light_comp_sp_ = hnll::game::point_light_component::create(0.0f, radius, color);
  set_renderable_component(light_comp_sp_);
  drag_comp_sp_ = draggable_component::create(get_transform_sp(), radius);
  set_translation(position);
}

control_point::control_point(const std::vector<s_ptr<control_point>> &base_points, const glm::vec3 &color, float radius)
  : hnll::game::actor()
{
  is_centroid_ = true;
  light_comp_sp_ = hnll::game::point_light_component::create(0.0f, radius, color);
  drag_comp_sp_ = draggable_component::create(get_transform_sp(), radius);

  // calc centroid position
  glm::vec3 pos;
  for (const auto &point: base_points)
    pos += point->get_transform_sp()->translation;
  pos /= base_points.size();
  set_translation(pos);
}

} // namespace iscg