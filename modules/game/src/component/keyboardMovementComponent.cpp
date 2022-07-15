#include <game/components/keyboardMovementComponent.hpp>
#include <game/engine.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// #include <X11/extensions/XTest.h>

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

keyboard_movement_component::key_mappings keyboard_movement_component::keys{};
keyboard_movement_component::PadMappings keyboard_movement_component::pads{};

keyboard_movement_component::keyboard_movement_component(GLFWwindow* window, transform& transform)
  : component(), window_(window), transform_(transform)
{
    // mapping
  const char* mapping = "03000000790000004618000010010000,Nintendo GameCube Controller Adapter,a:b1,b:b2,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,lefttrigger:b4,leftx:a0,lefty:a1,leftshoulder:b4,rightshoulder:b5,guide:b7,rightx:a5~,righty:a2~,start:b9,x:b0,y:b3,platform:Linux";
  glfwUpdateGamepadMappings(mapping);
  glfwSetJoystickCallback(joystickCallback);
  adjust_axis_errors();
}

void keyboard_movement_component::update_component(float dt)
{
  GLFWgamepadstate state;
  glfwGetGamepadState(pad_number_, &state);
  process_rotate_input(state, dt);
  process_move_input(state,dt);
  process_button_input(state, dt);
}

void keyboard_movement_component::process_rotate_input(GLFWgamepadstate& state, float dt)
{ 
  float rota_x = state.axes[pads.rota_x] 
              + (glfwGetKey(window_, keys.look_up) == GLFW_PRESS)
              - (glfwGetKey(window_, keys.look_down) == GLFW_PRESS);
  float rota_y = -state.axes[pads.rota_y]
              + (glfwGetKey(window_, keys.look_right) == GLFW_PRESS)
              - (glfwGetKey(window_, keys.look_left) == GLFW_PRESS);
  glm::vec3 rotate = {rota_x, rota_y, 0.f};
  // add to the current rotate matrix
  if (glm::dot(rotate, rotate) > ROTATE_THRESH) //std::numeric_limits<float>::epsilon())
    transform_.rotation += LOOK_SPEED * dt * glm::normalize(rotate);

  // limit pitch values between about +/- 58ish degrees
  transform_.rotation.x = glm::clamp(transform_.rotation.x, -1.5f, 1.5f);
  transform_.rotation.y = glm::mod(transform_.rotation.y, glm::two_pi<float>());
}

void keyboard_movement_component::process_move_input(GLFWgamepadstate& state, float dt)
{
  // camera translation
  // store user input
  float move_x = state.axes[pads.move_x] - right_error_;
  float move_y = -(state.axes[pads.move_y] - up_error_);
  float moveZ = state.buttons[pads.dp_up] - state.buttons[pads.dp_down];

  float yaw = transform_.rotation.y;
  const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
  const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
  const glm::vec3 upDir{0.f, -1.f, 0.f};

  glm::vec3 moveDir{0.f};
  float forward = move_y * (state.buttons[pads.left_bumper] == GLFW_PRESS)
    + (glfwGetKey(window_, keys.move_forward) == GLFW_PRESS)
    - (glfwGetKey(window_, keys.move_backward) == GLFW_PRESS);
  float right = move_x * (state.buttons[pads.left_bumper] == GLFW_PRESS)
    + (glfwGetKey(window_, keys.move_right) == GLFW_PRESS)
    - (glfwGetKey(window_, keys.move_left) == GLFW_PRESS);
  float up = (glfwGetKey(window_, keys.move_up) == GLFW_PRESS) - (glfwGetKey(window_, keys.move_down) == GLFW_PRESS)
    + moveZ;
  
  moveDir += sclXvec(forward, forwardDir) + sclXvec(right, rightDir) + sclXvec(up, upDir);

  if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
    transform_.translation += MOVE_SPEED * dt * glm::normalize(moveDir);

  // cursor move
  double xpos, ypos;
  glfwGetCursorPos(window_, &xpos, &ypos);
  xpos += move_x * CURSOR_SPEED * (state.buttons[pads.left_bumper] == GLFW_RELEASE);
  ypos -= move_y * CURSOR_SPEED * (state.buttons[pads.left_bumper] == GLFW_RELEASE);
  glfwSetCursorPos(window_, xpos, ypos);
}

void keyboard_movement_component::process_button_input(GLFWgamepadstate& state, float dt)
{
  // TODO : impl as lambda
  static bool isPressing = false;
  // Display* display;
  // click
  if (!isPressing && state.buttons[pads.button_a] == GLFW_PRESS) {
    // display = XOpenDisplay(NULL);
    // XTestFakeButtonEvent(display, 1, True, CurrentTime);
    // XFlush(display);
    // XCloseDisplay(display);
    isPressing = true;
  }
  // drag -> nothing to do
  else if (isPressing && state.buttons[pads.button_a] == GLFW_RELEASE) {
    // display = XOpenDisplay(NULL);
    // XTestFakeButtonEvent(display, 1, False, CurrentTime);
    // XFlush(display);
    // XCloseDisplay(display);
    isPressing = false;
  }
}

void keyboard_movement_component::adjust_axis_errors()
{
  GLFWgamepadstate state;
  glfwGetGamepadState(pad_number_, &state);
  right_error_ = state.axes[pads.move_x];
  up_error_ = state.axes[pads.move_y];
}


void keyboard_movement_component::set_default_mapping()
{
  // axis
  auto leftXfunc = [](float val){return glm::vec3(0.f, 0.f, 0.f); };
}

} // namespace hnll