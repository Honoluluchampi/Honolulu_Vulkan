#pragma once

#include <hge_component.hpp>
#include <utility.hpp>
#include <hve_renderer.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// implementation based on this tutorial https://www.youtube.com/watch?v=U0_ONQQ5ZNM&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=15

namespace hnll {

class ViewerComponent : public HgeComponent
{
  public:
    ViewerComponent(Transform& transform, HveRenderer& renderer);
    ~ViewerComponent() {}

    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
    void setPerspectiveProjection(float fovy, float aspect, float near, float far);

    // one of the three way to initialize view matrix
    // camera position, which direction to point, which direction is up
    void setViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up = glm::vec3(0.f, -1.f, 0.f));
    void setViewTarget(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3(0.f, -1.f, 0.f));
    void setViewYXZ();

    const glm::mat4& getProjection() const { return projectionMatrix_m; }
    const glm::mat4& getView() const { return viewMatrix_m; }

    void updateComponent(float dt) override;

  private:
    // ref of owner transform
    Transform& transform_m;
    glm::mat4 projectionMatrix_m{1.f};
    glm::mat4 viewMatrix_m{1.f};
    HveRenderer& hveRenderer_m;
};
} // namespace hnll