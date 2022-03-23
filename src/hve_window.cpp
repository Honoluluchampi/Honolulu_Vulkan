#include <hve_window.hpp>

namespace hve {

HvWindow::HvWindow(const int w, const int h, const std::string name) : width(w), height(h), windowName(name) 
{
  initWindow();
}

HvWindow::~HvWindow()
{
  glfwDestroyWindow(window);
  glfwTerminate();
}

void HvWindow::initWindow()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

} // namespace hv