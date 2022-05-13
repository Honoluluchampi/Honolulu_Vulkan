#include <GLFW/glfw3.h>
#include <iostream>

void error_callback(int error, const char* descritpion)
{ fprintf(stderr, "Error: %s\n", descritpion); }

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void joystickCallback(int jid, int event)
{
  std::cout << jid << std::endl;
  if (event == GLFW_CONNECTED) std::cout << "connected" << std::endl;
  else std::cout << "disconnected" << std::endl;
}

int main()
{
  if (!glfwInit()) return 0;
  glfwSetErrorCallback(error_callback);
  // create window
  GLFWwindow* window = glfwCreateWindow(800, 600, "joystick", NULL, NULL);
  if (!window)
    std::cout << "window creation failed" << std::endl;

  glfwSetKeyCallback(window, key_callback);
  glfwSetJoystickCallback(joystickCallback);

  // mapping
  const char* mapping = "03000000790000004618000010010000,Nintendo GameCube Controller Adapter,a:b1,b:b2,dpdown:b14,dpleft:b15,dpright:b13,dpup:b12,lefttrigger:b4,leftx:a0,lefty:a1,leftshoulder:b4,rightshoulder:b5,guide:b7,rightx:a5~,righty:a2~,start:b9,x:b0,y:b3,platform:Linux";
  glfwUpdateGamepadMappings(mapping);

  auto time = glfwGetTime();
  while(!glfwWindowShouldClose(window)) {
    while(glfwGetTime() - time < 1.0f) {}
    time = glfwGetTime();
    glfwPollEvents();

    int count;
    // const float* axis = glfwGetJoystickAxes(0, &count);
    // std::cout << count << std::endl;
    // // lx ly ry x x rx
    // for (int i = 0; i < count; i++) {
    //   std::cout << axis[i] << "  ";
    // }
    // std::cout << std::endl;

    // 0 -> 19
    // 0 x, 1 a, 2 b, 3 y, 4 l, 5 r, 7 z, 9 start, 
    // up 12(16), right 13(17), down 14(18), left 15(19) 
    // const unsigned char* button = glfwGetJoystickButtons(0, &count);
    // for (int i = 0; i < count; i++) {
    //   if (button[i] == GLFW_PRESS) std::cout << i << " press." << std::endl;
    // }

    GLFWgamepadstate state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
      if (state.buttons[GLFW_GAMEPAD_BUTTON_A])
        std::cout << "a" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_B])
        std::cout << "b" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_X])
        std::cout << "x" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_Y])
        std::cout << "y" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER])
        std::cout << "l bumper" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER])
        std::cout << "r bumper" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_START])
        std::cout << "start" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_BACK])
        std::cout << "back" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_GUIDE])
        std::cout << "guide" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP])
        std::cout << "dp up" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT])
        std::cout << "dp right" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT])
        std::cout << "dp left" << std::endl;
      if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN])
        std::cout << "dp down" << std::endl;
      
      std::cout << "lx : " << state.axes[GLFW_GAMEPAD_AXIS_LEFT_X] << std::endl;
      std::cout << "ly : " << state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] << std::endl;
      std::cout << "rx : " << state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] << std::endl;
      std::cout << "ry : " << state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] << std::endl;
    }
  }

  glfwDestroyWindow(window);
  glfwTerminate();
}