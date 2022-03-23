#include <hve_window.hpp>

namespace hve {

HveWindow::HveWindow(const int w, const int h, const std::string name) : width(w), height(h), windowName(name) 
{
  initWindow();
}

HveWindow::~HveWindow()
{
  glfwDestroyWindow(window);
  glfwTerminate();
}

void HveWindow::initWindow()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

} // namespace hv