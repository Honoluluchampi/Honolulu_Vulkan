#include <hge_components/keyboardMovementComponent.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace hnll {

void KeyboardMovementComponent::moveInPlaneXZ(float dt)
{
  // store user input
  glm::vec3 rotate{0};
  if (glfwGetKey(window_m, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
  if (glfwGetKey(window_m, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
  if (glfwGetKey(window_m, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
  if (glfwGetKey(window_m, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

  // if rotate vec is zero, dont change the rotateion matrix of gameObject
  if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
    transform_m.rotation_m += lookSpeed * dt * glm::normalize(rotate);

  // limit pitch values between about +/- 58ish degrees
  transform_m.rotation_m.x = glm::clamp(transform_m.rotation_m.x, -1.5f, 1.5f);
  transform_m.rotation_m.y = glm::mod(transform_m.rotation_m.y, glm::two_pi<float>());

  float yaw = transform_m.rotation_m.y;
  const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
  const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
  const glm::vec3 upDir{0.f, -1.f, 0.f};

  glm::vec3 moveDir{0.f};
  if (glfwGetKey(window_m, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
  if (glfwGetKey(window_m, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
  if (glfwGetKey(window_m, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
  if (glfwGetKey(window_m, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
  if (glfwGetKey(window_m, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
  if (glfwGetKey(window_m, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

  // // if move vec is zero, dont change the translation matrix of gameObject
  if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
    transform_m.translation_m += moveSpeed * dt * glm::normalize(moveDir);
}
} // namespace hnll