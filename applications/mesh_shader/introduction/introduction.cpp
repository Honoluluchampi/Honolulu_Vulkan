// hnll
#include <graphics/engine.hpp>
#include <graphics/utils.hpp>
#include <graphics/meshlet_model.hpp>
#include <game/actor.hpp>
#include <game/engine.hpp>
#include <game/components/meshlet_component.hpp>
#include <physics/engine.hpp>

// std
#include <memory>
#include <iostream>

namespace hnll {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;
using vec2 = Eigen::Vector2f;
using vec3 = Eigen::Vector3f;
using vec4 = Eigen::Vector4f;

class plane : public game::actor
{
  public:
    static s_ptr<plane> create(graphics::device& _device)
    {
      auto ret = std::make_shared<plane>();
      std::string filename = "bunny.obj";
      auto raw_meshlet_model = graphics::meshlet_model::create_from_file(_device, filename);
      ret->meshlet_comp_ = game::meshlet_component::create(ret, std::move(raw_meshlet_model));
      ret->set_rotation({M_PI, 0.f, 0.f});
      game::engine::add_actor(ret);
      return ret;
    }
    plane(){}
  private:
    s_ptr<game::meshlet_component> meshlet_comp_;
};

class mesh_shader_introduction : public game::engine
{
  public:
    mesh_shader_introduction() : game::engine("mesh shader introduction")
    {
      plane_ = plane::create(get_graphics_device());
    }

    ~mesh_shader_introduction() override = default;

  private:
    // sample object
    s_ptr<plane> plane_;
};
} // namespace hnll

int main() {
  hnll::mesh_shader_introduction app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}