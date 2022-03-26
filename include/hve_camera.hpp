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

    const glm::mat4& getProjection() const { return projectionMatrix_m; }
    
  private:
    glm::mat4 projectionMatrix_m{1.f};
};
} // namespace hve