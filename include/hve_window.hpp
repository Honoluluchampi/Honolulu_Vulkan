#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <string>

namespace hve {

class HveWindow
{
  public:
    HveWindow(const int w, const int h, const std::string name);
    ~HveWindow();

    // delete copy ctor, assignment (for preventing GLFWwindow* from double deleted)
    HveWindow(const HveWindow &) = delete;
    HveWindow& operator= (const HveWindow &) = delete;

    inline bool shouldClose() { return glfwWindowShouldClose(window); }
      
  private:
    void initWindow();

    const int width;
    const int height;

    std::string windowName;
    GLFWwindow *window;
};
    
} // namespace hv