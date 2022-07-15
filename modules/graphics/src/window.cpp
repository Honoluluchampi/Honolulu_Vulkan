// hnll
#include <graphics/window.hpp>

// std
#include <stdexcept>

namespace hnll {

window::window(const int w, const int h, const std::string name) : width_(w), height_(h), window_name_(name) 
{
  init_window();
}

window::~window()
{
  glfwDestroyWindow(window_);
  glfwTerminate();
}

void window::init_window()
{
  glfwInit();
  // disable openGL
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window_ = glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);
  // handle framebuffer resizes
  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(window_, frame_buffer_resize_callback);
}

void window::create_window_surface(VkInstance instance, VkSurfaceKHR* surface)
{
  // glfwCreateWindowSurface is implemented to multi-platfowm
  // we dont have to implement create_surface function using platform-specific extension
  if (glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS)  
    throw std::runtime_error("failed to create window surface");
}

void window::frame_buffer_resize_callback(GLFWwindow *window, int width, int height)
{
  auto hveWindow = reinterpret_cast<window *>(glfwGetWindowUserPointer(window));
  hveWindow->frame_buffer_resized_ = true;
  hveWindow->width_ = width;
  hveWindow->height_ = height;
}

} // namespace hv