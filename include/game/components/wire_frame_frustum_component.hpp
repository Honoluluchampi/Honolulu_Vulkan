#pragma once

// hnll
#include <game/components/renderable_component.hpp>
#include <graphics/mesh_model.hpp>
// TODO : make graphics::device static
#include <graphics/device.hpp>
#include <geometry/perspective_frustum.hpp>

namespace hnll {

namespace game {

class wire_frame_frustum_component : public renderable_component
{
  public:
    template <Actor A>
    static s_ptr<wire_frame_frustum_component> create(s_ptr<A>& owner_sp, s_ptr<geometry::perspective_frustum>& frustum, graphics::device& device)
    {
      auto wire_frustum = std::make_shared<wire_frame_frustum_component>(owner_sp);
      wire_frustum->set_perspective_frustum(frustum);
      wire_frustum->create_frustum_mesh_model(device);
      owner_sp->set_renderable_component(wire_frustum);
      return wire_frustum;
    }
    template <Actor A>
    wire_frame_frustum_component(s_ptr<A>& owner_sp) : renderable_component(owner_sp, render_type::WIRE_FRUSTUM) {}
    ~wire_frame_frustum_component() override = default;

    // getter
    const geometry::perspective_frustum& get_perspective_frustum() const { return *frustum_; }
    const s_ptr<graphics::mesh_model>&   get_frustum_mesh_sp()     const { return frustum_mesh_; }
    vec3  get_color() const { return color_; }
    // setter
    void set_perspective_frustum(s_ptr<geometry::perspective_frustum>& p_frustum) { frustum_ = p_frustum; }
  private:
    s_ptr<geometry::perspective_frustum> frustum_ = nullptr;
    vec3 color_ = { 0.8f, 0.2f, 0.1f };
    s_ptr<graphics::mesh_model> frustum_mesh_;

    // TODO : compute mesh vertices only in this function
    void create_frustum_mesh_model(graphics::device& device)
    {
      const auto& default_points = frustum_->get_default_points();
      auto near = frustum_->get_near_z();
      auto far  = frustum_->get_far_z();

      // register vertices
      std::vector<graphics::mesh_model::vertex> vertices;
      graphics::mesh_model::vertex vertex;
      vertex.color    = color_.cast<float>();
      vertex.position = near * default_points[0].cast<float>();
      vertex.uv = {0, 0};
      vertices.push_back(vertex);
      vertex.position = near * default_points[1].cast<float>();
      vertex.uv = {1, 0};
      vertices.push_back(vertex);
      vertex.position = near * default_points[2].cast<float>();
      vertex.uv = {0, 0};
      vertices.push_back(vertex);
      vertex.position = near * default_points[3].cast<float>();
      vertex.uv = {0, 1};
      vertices.push_back(vertex);
      vertex.position = far  * default_points[0].cast<float>();
      vertex.uv = {0, 1};
      vertices.push_back(vertex);
      vertex.position = far  * default_points[1].cast<float>();
      vertex.uv = {0, 0};
      vertices.push_back(vertex);
      vertex.position = far  * default_points[2].cast<float>();
      vertex.uv = {1, 0};
      vertices.push_back(vertex);
      vertex.position = far  * default_points[3].cast<float>();
      vertex.uv = {0, 0};
      vertices.push_back(vertex);
      vertex.position = near * default_points[3].cast<float>();
      vertex.uv = {1, 0};
      vertices.push_back(vertex);
      vertex.position = far  * default_points[2].cast<float>();
      vertex.uv = {0, 1};
      vertices.push_back(vertex);
      // register indices
      std::vector<uint32_t> indices = {
          0, 3, 1,
          1, 3, 2,
          1, 2, 9,
          5, 1, 9,
          4, 5, 6,
          4, 6, 7,
          0, 4, 8,
          4, 7, 8,
          0, 1, 4,
          1, 5, 4,
          2, 3, 6,
          7, 6, 3,
      };

      graphics::mesh_model::builder builder;
      builder.vertices = std::move(vertices);
      builder.indices  = std::move(indices);
      frustum_mesh_ = std::make_shared<graphics::mesh_model>(device, builder);
    }
};

}} // namespace hnll::game