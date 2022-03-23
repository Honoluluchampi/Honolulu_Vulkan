#include <hve_app.hpp>

namespace hve {

void HveApp::run()
{
  while (!hveWindow_m.shouldClose()) {
    glfwPollEvents();
  }
}

} // namespace hve