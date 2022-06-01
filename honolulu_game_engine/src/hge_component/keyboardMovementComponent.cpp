#include <hge_components/keyboardMovementComponent.hpp>
#include <hge_game.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <X11/extensions/XTest.h>

void joystickCallback(int jid, int event)
{
  std::cout << jid << std::endl;
  if (event == GLFW_CONNECTED) std::cout << "connected" << std::endl;
  else std::cout << "disconnected" << std::endl;
}

namespace hnll {

constexpr float MOVE_THRESH = 0.1f; 
constexpr float ROTATE_THRESH = 0.1f;

constexpr float MOVE_SPEED = 5.f;
constexpr float LOOK_SPEED = 1.5f;
constexpr float CURSOR_SPEED = 15.f;

KeyboardMovementComponent::KeyMappings KeyboardMovementComponent::keys{};
KeyboardMovementComponent::PadMappings KeyboardMovementComponent::pads{};

KeyboardMovementComponent::KeyboardMovementComponent(GLFWwindow* window, Transform& transform)
  : HgeComponent(), window_m(window), transform_m(transform)
{
    // mapping
  const char* mapping = "03000000790000004618000010010000,Nintendo GameCube Controller Adapter,a:b1,b:b2,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,lefttrigger:b4,leftx:a0,lefty:a1,leftshoulder:b4,rightshoulder:b5,guide:b7,rightx:a5~,righty:a2~,start:b9,x:b0,y:b3,platform:Linux";
  glfwUpdateGamepadMappings(mapping);
  glfwSetJoystickCallback(joystickCallback);
  adjustAxisErrors();
}

void KeyboardMovementComponent::updateComponent(float dt)
{
  GLFWgamepadstate state;
  glfwGetGamepadState(padId, &state);
  processRotateInput(state, dt);
  processMoveInput(state,dt);
  processButtonInput(state, dt);
}

void KeyboardMovementComponent::processRotateInput(GLFWgamepadstate& state, float dt)
{ 
  float rotaX = state.axes[pads.rotaX] 
              + (glfwGetKey(window_m, keys.lookUp) == GLFW_PRESS)
              - (glfwGetKey(window_m, keys.lookDown) == GLFW_PRESS);
  float rotaY = -state.axes[pads.rotaY]
              + (glfwGetKey(window_m, keys.lookRight) == GLFW_PRESS)
              - (glfwGetKey(window_m, keys.lookLeft) == GLFW_PRESS);
  glm::vec3 rotate = {rotaX, rotaY, 0.f};
  // add to the current rotate matrix
  if (glm::dot(rotate, rotate) > ROTATE_THRESH) //std::numeric_limits<float>::epsilon())
    transform_m.rotation_m += LOOK_SPEED * dt * glm::normalize(rotate);

  // limit pitch values between about +/- 58ish degrees
  transform_m.rotation_m.x = glm::clamp(transform_m.rotation_m.x, -1.5f, 1.5f);
  transform_m.rotation_m.y = glm::mod(transform_m.rotation_m.y, glm::two_pi<float>());
}

void KeyboardMovementComponent::processMoveInput(GLFWgamepadstate& state, float dt)
{
  // camera translation
  // store user input
  float moveX = state.axes[pads.moveX] - rightError_;
  float moveY = -(state.axes[pads.moveY] - upError_);
  float moveZ = state.buttons[pads.dpUp] - state.buttons[pads.dpDown];

  float yaw = transform_m.rotation_m.y;
  const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
  const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
  const glm::vec3 upDir{0.f, -1.f, 0.f};

  glm::vec3 moveDir{0.f};
  float forward = moveY * (state.buttons[pads.leftBumper] == GLFW_PRESS)
    + (keys.moveForward == GLFW_PRESS)
    - (keys.moveBackward == GLFW_PRESS);
  float right = moveX * (state.buttons[pads.leftBumper] == GLFW_PRESS)
    + (keys.moveRight == GLFW_PRESS)
    - (keys.moveLeft == GLFW_PRESS);
  float up = (keys.moveUp == GLFW_PRESS) - (keys.moveDown == GLFW_PRESS)
    + moveZ;
  
  moveDir += sclXvec(forward, forwardDir) + sclXvec(right, rightDir) + sclXvec(up, upDir);

  if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
    transform_m.translation_m += MOVE_SPEED * dt * glm::normalize(moveDir);

  // cursor move
  double xpos, ypos;
  glfwGetCursorPos(window_m, &xpos, &ypos);
  xpos += moveX * CURSOR_SPEED * (state.buttons[pads.leftBumper] == GLFW_RELEASE);
  ypos -= moveY * CURSOR_SPEED * (state.buttons[pads.leftBumper] == GLFW_RELEASE);
  glfwSetCursorPos(window_m, xpos, ypos);
}

void KeyboardMovementComponent::processButtonInput(GLFWgamepadstate& state, float dt)
{
  // TODO : impl as lambda
  static bool isPressing = false;
  Display* display;
  // click
  if (!isPressing && state.buttons[pads.buttonA] == GLFW_PRESS) {
    display = XOpenDisplay(NULL);
    XTestFakeButtonEvent(display, 1, True, CurrentTime);
    XFlush(display);
    XCloseDisplay(display);
    isPressing = true;
  }
  // drag -> nothing to do
  else if (isPressing && state.buttons[pads.buttonA] == GLFW_RELEASE) {
    display = XOpenDisplay(NULL);
    XTestFakeButtonEvent(display, 1, False, CurrentTime);
    XFlush(display);
    XCloseDisplay(display);
    isPressing = false;
  }
}

void KeyboardMovementComponent::adjustAxisErrors()
{
  GLFWgamepadstate state;
  glfwGetGamepadState(padId, &state);
  rightError_ = state.axes[pads.moveX];
  upError_ = state.axes[pads.moveY];
}


void KeyboardMovementComponent::setDefaultMapping()
{
  // axis
  auto leftXfunc = [](float val){return glm::vec3(0.f, 0.f, 0.f); };
}

} // namespace hnll