#include <hve_window.hpp>

// std
#include <stdexcept>

namespace hnll {

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
  // disable openGL
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window_m = glfwCreateWindow(width_m, height_m, windowName_m.c_str(), nullptr, nullptr);
  // handle framebuffer resizes
  glfwSetWindowUserPointer(window_m, this);
  glfwSetFramebufferSizeCallback(window_m, framebufferResizeCallBack);
}

void HveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
  // glfwCreateWindowSurface is implemented to multi-platfowm
  // we dont have to implement createSurface function using platform-specific extension
  if (glfwCreateWindowSurface(instance, window_m, nullptr, surface) != VK_SUCCESS)  
    throw std::runtime_error("failed to create window surface");
}

void HveWindow::framebufferResizeCallBack(GLFWwindow *window, int width, int height)
{
  auto hveWindow = reinterpret_cast<HveWindow *>(glfwGetWindowUserPointer(window));
  hveWindow->framebufferResized_m = true;
  hveWindow->width_m = width;
  hveWindow->height_m = height;
}

} // namespace hv