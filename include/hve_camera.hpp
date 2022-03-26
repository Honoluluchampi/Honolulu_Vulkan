#pragma once

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// implementation based on this tutorial https://www.youtube.com/watch?v=U0_ONQQ5ZNM&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=15

namespace hve {

class HveCamera 
{
  public:
    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

    void setPerspectiveProjection(float fovy, float aspect, float near, float far);

    // one of the three way to initialize view matrix
    // camera position, which direction to point, which direction is up
    void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f)); 
    void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f)); 
    void setViewYXZ(glm::vec3 position, glm::vec3 rotation);


    const glm::mat4& getProjection() const { return projectionMatrix_m; }
    const glm::mat4& getView() const { return viewMatrix_m; }
    
  private:
    glm::mat4 projectionMatrix_m{1.f};
    glm::mat4 viewMatrix_m{1.f};
};
} // namespace hve