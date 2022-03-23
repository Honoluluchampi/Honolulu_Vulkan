#include <hve_app.hpp>

namespace hve {

void HveApp::run()
{
  while (!hveWindow.shouldClose()) {
    glfwPollEvents();
  }
}

} // namespace hve