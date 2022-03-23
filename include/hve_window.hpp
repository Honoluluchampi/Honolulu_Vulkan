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

    inline bool shouldClose() { return glfwWindowShouldClose(window_m); }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
      
  private:
    void initWindow();

    const int width_m;
    const int height_m;

    std::string windowName_m;
    GLFWwindow *window_m;
};
    
} // namespace hv