#pragma once

#include <hge_component.hpp>
#include <utility.hpp>

// lib
#include <GLFW/glfw3.h>

namespace hnll {

class KeyboardMovementComponent : public HgeComponent
{
  public:
    KeyboardMovementComponent(GLFWwindow* window, Transform& transform) 
      : HgeComponent(), window_m(window), transform_m(transform){}
    
    struct KeyMappings 
    {
      int moveLeft = GLFW_KEY_A;
      int moveRight = GLFW_KEY_D;
      int moveForward = GLFW_KEY_W;
      int moveBackward = GLFW_KEY_S;
      int moveUp = GLFW_KEY_E;
      int moveDown = GLFW_KEY_Q;
      int lookLeft = GLFW_KEY_LEFT;
      int lookRight = GLFW_KEY_RIGHT;
      int lookUp = GLFW_KEY_UP;
      int lookDown = GLFW_KEY_DOWN;
    };

    // update owner's position (owner's transformation's ref was passed in the ctor)
    inline void updateComponent(float dt) override
    { moveInPlaneXZ(dt); }

    void moveInPlaneXZ(float dt);

  private:
    KeyMappings keys{};
    GLFWwindow* window_m;
    // this component should be delete before the owner
    Transform& transform_m;
    float moveSpeed{3.f};
    float lookSpeed{1.5f};
};

} // namespace hnll