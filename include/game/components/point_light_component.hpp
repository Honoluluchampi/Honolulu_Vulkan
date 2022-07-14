#pragma once

#include <game/components/renderable_component.hpp>

namespace hnll {

// pointLight can be treated as gameObject
struct PointLightInfo
{
  float lightIntensity_m = 1.0f;
};

class PointLightComponent : public HgeRenderableComponent
{
  public:
    PointLightComponent(actorId id);
    ~PointLightComponent(){}

    // setter
    void setRadius(float radius) 
    { this->setScale(glm::vec3(radius, radius, radius)); }
    // getter
    PointLightInfo& getLightInfo() { return lightInfo_m; }
    glm::vec3& getColor() { return color_m; }
    static s_ptr<PointLightComponent> createPointLight
      (id_t id, float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));
  private:
    PointLightComponent(const PointLightComponent&) = delete;
    PointLightComponent& operator=(const PointLightComponent&) = delete;
    PointLightInfo lightInfo_m{};
    glm::vec3 color_m{};
};

}