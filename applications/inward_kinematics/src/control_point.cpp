// iscg
#include <control_point.hpp>

namespace iscg {

control_point::control_point(const glm::vec3 &position, const glm::vec3 &color, float radius)
  : hnll::game::actor()
{
  light_comp_sp_ = hnll::game::point_light_component::create_point_light(0.0f, radius, color);
  light_comp_sp_->get_transform().translation = position;
  drag_comp_sp_ = std::make_shared<DraggableComponent>(lightComp_->getTransform(), radius);
}

control_point::control_point(const std::vector <s_ptr<ControllPoint>> &basePoints, const glm::vec3 &color, float radius)
  : hnll::game::actor()
{
  isCentroid_ = true;
  lightComp_ = hnll::PointLightComponent::createPointLight(this->getId(), 0.0f, radius, color);
  // calc centroid position
  glm::vec3 pos;
  for (const auto &point: basePoints)
    pos += point->lightComp()->getTransform().translation_m;
  pos /= basePoints.size();
  lightComp_->getTransform().translation_m = pos;
  dragComp_ = std::make_shared<DraggableComponent>(lightComp_->getTransform(), radius);
}

} // namespace iscg