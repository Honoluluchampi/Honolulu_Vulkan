#pragma once

// hnll
#include <game/component.hpp>
#include <utils/utils.hpp>

// std
#include <unordered_map>

// lib
#include <GLFW/glfw3.h>

namespace hnll {

class KeyboardMovementComponent : public HgeComponent
{
  public:
    using KeyId = int;
    using PadId = int;
    using ButtonMap = std::unordered_map<KeyId, u_ptr<std::function<void()>>>;
    using PadMap = std::unordered_map<PadId, u_ptr<std::function<glm::vec3(float,float)>>>;
    
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

    struct PadMappings
    {
      PadId buttonA = GLFW_GAMEPAD_BUTTON_A;
      PadId buttonB = GLFW_GAMEPAD_BUTTON_B;
      PadId buttonY = GLFW_GAMEPAD_BUTTON_Y;
      PadId buttonX = GLFW_GAMEPAD_BUTTON_X;
      PadId buttonGuide = GLFW_GAMEPAD_BUTTON_GUIDE;
      PadId buttonStart = GLFW_GAMEPAD_BUTTON_START;
      PadId leftBumper = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER;
      PadId rightBumper = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER;
      PadId dpUp = GLFW_GAMEPAD_BUTTON_DPAD_UP;
      PadId dpDown = GLFW_GAMEPAD_BUTTON_DPAD_DOWN;
      PadId moveX = GLFW_GAMEPAD_AXIS_LEFT_X;
      PadId moveY = GLFW_GAMEPAD_AXIS_LEFT_Y;
      PadId rotaX = GLFW_GAMEPAD_AXIS_RIGHT_Y;
      PadId rotaY = GLFW_GAMEPAD_AXIS_RIGHT_X;
    };

    // update owner's position (owner's transformation's ref was passed in the ctor)
    void updateComponent(float dt) override;

    // dont use lambda's capture
    // TODO : check whether using checkingButtonList fasten the checking sequence
    void setButtonFunc(KeyId keyId, std::function<void()> func)
    { buttonMap_.emplace(keyId, std::make_unique<std::function<void()>>(func)); }
    // dont use lambda's capture
    void setAxisFunc(PadId axisId, std::function<glm::vec3(float, float)> func)
    { padMap_.emplace(axisId, std::make_unique<std::function<glm::vec3(float, float)>>(func)); }

    void removeButtonFunc(KeyId keyId)
    { buttonMap_.erase(keyId); }
    void removeAxisFunc(PadId axisId)
    { padMap_.erase(axisId); }

  private:
    void adjustAxisErrors();
    void setDefaultMapping();
    void processRotateInput(GLFWgamepadstate& state, float dt);
    void processMoveInput(GLFWgamepadstate& state, float dt);
    void processButtonInput(GLFWgamepadstate& state, float dt);

    // mapping
    static KeyMappings keys;
    static PadMappings pads;
    
    GLFWwindow* window_m;
    // this component should be deleted before the owner
    Transform& transform_m;

    ButtonMap buttonMap_;
    // funcs take GLFWgamepadstate.axes[glfw_gamepad_axis_leftorright_xory], dt
    PadMap padMap_;
    int padId = GLFW_JOYSTICK_1;
    // to adjust default input
    float rightError_ = 0.f, upError_ = 0.f;
};

} // namespace hnll