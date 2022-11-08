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
      std::string filename = "/home/honolulu/models/primitives/sphere.obj";
      auto raw_meshlet_model = graphics::meshlet_model::create_from_file(_device, filename);
//      auto raw_meshlet_model = create_raw_meshlets(_device);
      ret->meshlet_comp_ = game::meshlet_component::create(ret, std::move(raw_meshlet_model));
      ret->set_translation({0.f, -1.f, 0.f});
      game::engine::add_actor(ret);
      return ret;
    }
    plane(){}
  private:
    static s_ptr<graphics::meshlet_model> create_raw_meshlets(graphics::device& _device)
    {
      // v3 --- v2
      //  |  /  |
      // v0 --- v1
      graphics::vertex v0 = { vec3{-0.5f,  0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 1.f, 0.f}, vec2{0.f, 0.f} };
      graphics::vertex v1 = { vec3{ 0.5f,  0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{1.f, 0.f, 0.f}, vec2{0.f, 0.f} };
      graphics::vertex v2 = { vec3{ 0.5f, -0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 1.f, 0.f}, vec2{0.f, 0.f} };
      graphics::vertex v3 = { vec3{-0.5f, -0.5f, 0.f}, vec3{0.f, -1.f, 0.f}, vec3{0.f, 0.f, 1.f}, vec2{0.f, 0.f} };
      std::vector<graphics::vertex> raw_vertices = { v0, v1, v2, v3 };

      std::vector<graphics::meshlet> meshlets = {
        {{0, 1, 2}, {0, 1, 2}, 3, 3},
        {{0, 2, 3}, {0, 1, 2}, 3, 3},
      };

      return graphics::meshlet_model::create(_device, std::move(raw_vertices), std::move(meshlets));
    }
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