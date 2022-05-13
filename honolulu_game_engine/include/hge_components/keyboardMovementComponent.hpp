#pragma once

#include <hge_component.hpp>
#include <utility.hpp>

// lib
#include <GLFW/glfw3.h>

namespace hnll {

class KeyboardMovementComponent : public HgeComponent
{
  public:
    using KeyId = int;
    using AxisId = int;
    using ButtonMap = std::unordered_map<KeyId, u_ptr<std::function<void()>>>;
    using AxisMap = std::unordered_map<AxisId, u_ptr<std::function<glm::vec3(float,float)>>>;
    
    KeyboardMovementComponent(GLFWwindow* window, Transform& transform);
    
    struct KeyMappings 
    {
      KeyId moveLeft = GLFW_KEY_A;
      KeyId moveRight = GLFW_KEY_D;
      KeyId moveForward = GLFW_KEY_W;
      KeyId moveBackward = GLFW_KEY_S;
      KeyId moveUp = GLFW_KEY_E;
      KeyId moveDown = GLFW_KEY_Q;
      KeyId lookLeft = GLFW_KEY_LEFT;
      KeyId lookRight = GLFW_KEY_RIGHT;
      KeyId lookUp = GLFW_KEY_UP;
      KeyId lookDown = GLFW_KEY_DOWN;
    };

    struct AxisMappings
    {
      AxisId moveX = GLFW_GAMEPAD_AXIS_LEFT_X;
      AxisId moveY = GLFW_GAMEPAD_AXIS_LEFT_X;
      AxisId rotaX = GLFW_GAMEPAD_AXIS_RIGHT_Y;
      AxisId rotaY = GLFW_GAMEPAD_AXIS_RIGHT_X;
    };

    // update owner's position (owner's transformation's ref was passed in the ctor)
    void updateComponent(float dt) override;

    void moveInPlaneXZ(float dt);

    // dont use lambda's capture
    // TODO : check whether using checkingButtonList fasten the checking sequence
    void setButtonFunc(KeyId keyId, std::function<void()> func)
    { buttonMap_.emplace(keyId, std::make_unique<std::function<void()>>(func)); }
    // dont use lambda's capture
    void setAxisFunc(AxisId axisId, std::function<glm::vec3(float, float)> func)
    { axisMap_.emplace(axisId, std::make_unique<std::function<glm::vec3(float, float)>>(func)); }

    void removeButtonFunc(KeyId keyId)
    { buttonMap_.erase(keyId); }
    void removeAxisFunc(AxisId axisId)
    { axisMap_.erase(axisId); }

  private:
    void setDefaultMapping();
    void processRotateInput(GLFWgamepadstate& state, float dt);
    void processMoveInput(GLFWgamepadstate& state, float dt){}
    void processButtonInput(GLFWgamepadstate& state, float dt){}

    // mapping
    static KeyMappings keys;
    static AxisMappings axis;
    
    GLFWwindow* window_m;
    // this component should be deleted before the owner
    Transform& transform_m;
    float moveSpeed{3.f};
    float lookSpeed{1.5f};

    ButtonMap buttonMap_;
    // funcs take GLFWgamepadstate.axes[glfw_gamepad_axis_leftorright_xory], dt
    AxisMap axisMap_;
    int padId = GLFW_JOYSTICK_1;
};

} // namespace hnll