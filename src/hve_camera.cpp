#include <hve_camera.hpp>

// std
#include <cassert>
#include <limits>

namespace hve {

void HveCamera::setOrthographicProjection(
    float left, float right, float top, float bottom, float near, float far) {
  projectionMatrix_m = glm::mat4{1.0f};
  projectionMatrix_m[0][0] = 2.f / (right - left);
  projectionMatrix_m[1][1] = 2.f / (bottom - top);
  projectionMatrix_m[2][2] = 1.f / (far - near);
  projectionMatrix_m[3][0] = -(right + left) / (right - left);
  projectionMatrix_m[3][1] = -(bottom + top) / (bottom - top);
  projectionMatrix_m[3][2] = -near / (far - near);
}
 
void HveCamera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
  assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
  const float tanHalfFovy = tan(fovy / 2.f);
  projectionMatrix_m = glm::mat4{0.0f};
  projectionMatrix_m[0][0] = 1.f / (aspect * tanHalfFovy);
  projectionMatrix_m[1][1] = 1.f / (tanHalfFovy);
  projectionMatrix_m[2][2] = far / (far - near);
  projectionMatrix_m[2][3] = 1.f;
  projectionMatrix_m[3][2] = -(far * near) / (far - near);
}
} // namesapce hve