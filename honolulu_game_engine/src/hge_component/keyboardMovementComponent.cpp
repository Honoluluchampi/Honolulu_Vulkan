#include <hge_components/keyboardMovementComponent.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

constexpr float MOVE_THRESH = 0.1f; 
constexpr float ROTATE_THRESH = 0.1f;

namespace hnll {

KeyboardMovementComponent::KeyMappings KeyboardMovementComponent::keys{};
KeyboardMovementComponent::AxisMappings KeyboardMovementComponent::axis{};

KeyboardMovementComponent::KeyboardMovementComponent(GLFWwindow* window, Transform& transform)
  : HgeComponent(), window_m(window), transform_m(transform)
{
    // mapping
  const char* mapping = "03000000790000004618000010010000,Nintendo GameCube Controller Adapter,a:b1,b:b2,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,lefttrigger:b4,leftx:a0,lefty:a1,leftshoulder:b4,rightshoulder:b5,guide:b7,rightx:a5~,righty:a2~,start:b9,x:b0,y:b3,platform:Linux";
  glfwUpdateGamepadMappings(mapping);
}

void KeyboardMovementComponent::updateComponent(float dt)
{
  GLFWgamepadstate state;
  glfwGetGamepadState(padId, &state);
  processRotateInput(state, dt);
  // temp
  moveInPlaneXZ(dt);
  processMoveInput(state,dt);
  processButtonInput(state, dt);
}

void KeyboardMovementComponent::processRotateInput(GLFWgamepadstate& state, float dt)
{
  // store user input
  float moveX = state.axes[axis.moveX];
  float moveY = state.axes[axis.moveY];
  
  float rotaX = state.axes[axis.rotaX] 
              + (glfwGetKey(window_m, keys.lookUp) == GLFW_PRESS)
              - (glfwGetKey(window_m, keys.lookDown) == GLFW_PRESS);
  float rotaY = state.axes[axis.rotaY]
              + (glfwGetKey(window_m, keys.lookRight) == GLFW_PRESS)
              - (glfwGetKey(window_m, keys.lookLeft) == GLFW_PRESS);
  glm::vec3 rotate = {rotaX, rotaY, 0.f};
  // add to the current rotate matrix
  if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
    transform_m.rotation_m += lookSpeed * dt * glm::normalize(rotate);

  // limit pitch values between about +/- 58ish degrees
  transform_m.rotation_m.x = glm::clamp(transform_m.rotation_m.x, -1.5f, 1.5f);
  transform_m.rotation_m.y = glm::mod(transform_m.rotation_m.y, glm::two_pi<float>());
}

void KeyboardMovementComponent::moveInPlaneXZ(float dt)
{
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

void KeyboardMovementComponent::setDefaultMapping()
{
  // axis
  auto leftXfunc = [](float val){return glm::vec3(0.f, 0.f, 0.f); };
}

} // namespace hnll