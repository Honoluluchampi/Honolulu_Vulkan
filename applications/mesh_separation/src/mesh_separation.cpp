// hnll
#include <game/engine.hpp>
#include <geometry/mesh_model.hpp>
#include <geometry/half_edge.hpp>

using namespace hnll;

class app
{
  public:
    app() : engine_(std::make_unique<game::engine>("mesh_separation"))
    {}

  private:
    u_ptr<game::engine> engine_;
    s_ptr<geometry::mesh_model> plane_geometry_;
    s_ptr<graphics::mesh_model> plane_graphics_;
};