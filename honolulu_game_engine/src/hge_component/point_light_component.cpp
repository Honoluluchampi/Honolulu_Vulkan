#include <hge_components/point_light_component.hpp>

namespace hnll {

PointLightComponent::PointLightComponent(id_t id)
 : HgeRenderableComponent(id, RenderType::POINT_LIGHT)
{}

s_ptr<PointLightComponent> PointLightComponent::createPointLight
  (id_t id, float intensity, float radius, glm::vec3 color)
{
  auto light = std::make_shared<PointLightComponent>(id);
  light->color_m = color;
  light->setScale(glm::vec3(radius, radius, radius));
  light->lightInfo_m.lightIntensity_m = intensity;
  return std::move(light);
}

} // namespace hnll