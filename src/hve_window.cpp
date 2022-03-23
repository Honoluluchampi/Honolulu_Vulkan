#include <hve_window.hpp>

// std
#include <stdexcept>

namespace hve {

HveWindow::HveWindow(const int w, const int h, const std::string name) : width_m(w), height_m(h), windowName_m(name) 
{
  initWindow();
}

HveWindow::~HveWindow()
{
  glfwDestroyWindow(window_m);
  glfwTerminate();
}

void HveWindow::initWindow()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window_m = glfwCreateWindow(width_m, height_m, windowName_m.c_str(), nullptr, nullptr);
}

void HveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
  // glfwCreateWindowSurface is implemented to multi-platfowm
  // we dont have to implement createSurface function using platform-specific extension
  if (glfwCreateWindowSurface(instance, window_m, nullptr, surface) != VK_SUCCESS)  
    throw std::runtime_error("failed to create window surface");
}

} // namespace hv