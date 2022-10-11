// hnll
#include <game/engine.hpp>
#include <game/components/viewer_component.hpp>
#include <game/components/rigid_component.hpp>
#include <game/components/wire_frame_frustum_component.hpp>
#include <geometry/mesh_separation.hpp>

namespace hnll {

class virtual_camera : public game::actor
{
  public:
    static s_ptr<virtual_camera> create(hnll::graphics::renderer& renderer)
    {
      auto camera = std::make_shared<virtual_camera>();
      auto viewer_comp = game::viewer_component::create(*camera->get_transform_sp(), renderer);
      viewer_comp->auto_update_view_frustum();
    }
    virtual_camera() = default;
    ~virtual_camera() = default;
  private:
    u_ptr<game::viewer_component> viewer_comp_;
    u_ptr<game::wire_frame_frustum_component> wire_frustum_comp_;
};

class view_frustum_culling : public game::engine
{
  public:
    view_frustum_culling() : game::engine("view_frustum_culling")
    {

    }

    void update_game(float dt) override
    {

    }
  private:
    void add_virtual_camera()
    {
      auto virtual_camera = virtual_camera::create(get_graphics_engine().get_renderer());
      add_actor(virtual_camera);
    }

    void add_separated_model(const std::string& filename)
    {

    }
};

}

int main()
{
  hnll::view_frustum_culling app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}