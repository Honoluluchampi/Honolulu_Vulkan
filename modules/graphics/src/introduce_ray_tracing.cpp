#include <graphics/device.hpp>

namespace hnll {

class hello_triangle
{
  public:
    hello_triangle()
    {
      device_.set_rendering_type(graphics::rendering_type::RAY_TRACING);
    }

  private:
    graphics::window window_{1920, 1080, "hello ray tracing triangle"};
    graphics::device device_{window_};
};
}

int main() {
  hnll::hello_triangle app;
}

// empty
#include <geometry/mesh_model.hpp>
void hnll::geometry::mesh_model::align_vertex_id() {}
