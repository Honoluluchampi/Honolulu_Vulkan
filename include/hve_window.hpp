#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <string>

namespace hve {

class HvWindow
{
  public:
    HvWindow(const int w, const int h, const std::string name);
    ~HvWindow();
      
  private:
    void initWindow();

    const int width;
    const int height;

    std::string windowName;
    GLFWwindow *window;
};
    
} // namespace hv