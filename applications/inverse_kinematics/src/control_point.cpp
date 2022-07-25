// iscg
#include <control_point.hpp>

namespace iscg {

s_ptr<control_point> control_point::create(const glm::vec3 &position, const glm::vec3 &color, float radius)
{
  auto cp = std::make_shared<control_point>(position, color, radius);
  auto light_comp_sp = hnll::game::point_light_component::create(cp, 0.0f, radius, color);
  auto drag_comp_sp = draggable_component::create(cp->get_transform_sp(), radius);
  cp->set_light_comp_sp(light_comp_sp);
  cp->set_drag_comp_sp(drag_comp_sp);
  cp->set_translation(position);
  return cp;
}

control_point::control_point(const glm::vec3 &position, const glm::vec3 &color, float radius)
  : hnll::game::actor()
{}

//control_point::control_point(const std::vector<s_ptr<control_point>> &base_points, const glm::vec3 &color, float radius)
//  : hnll::game::actor()
//{
//  is_centroid_ = true;
//  light_comp_sp_ = hnll::game::point_light_component::create(0.0f, radius, color);
//  drag_comp_sp_ = draggable_component::create(get_transform_sp(), radius);
//
//  // calc centroid position
//  glm::vec3 pos;
//  for (const auto &point: base_points)
//    pos += point->get_transform_sp()->translation;
//  pos /= base_points.size();
//  set_translation(pos);
//}

} // namespace iscg