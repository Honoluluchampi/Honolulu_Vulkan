#pragma once
// hnll
#include <game/component.hpp>
#include <geometry/bounding_volume.hpp>

namespace hnll {

namespace game {

class rigid_component : public component
{
  public:
    rigid_component() : component() {}
    rigid_component(geometry::bounding_volume&& bv)
      : component(), bounding_volume_(std::make_unique<geometry::bounding_volume>(bv)){}

    // bounding_volume should be given as r-value reference
    static u_ptr<rigid_component> create(geometry::bounding_volume&& bv)
    {
      auto rc = std::make_unique<rigid_component>(std::move(bv));
      return rc;
    }

    static u_ptr<rigid_component> create(const std::vector<vec3>& positions, geometry::bv_type type)
    {
      auto rc = std::make_unique<rigid_component>();
      u_ptr<geometry::bounding_volume> bv;
      if (type == geometry::bv_type::AABB)
        bv = geometry::bounding_volume::create_aabb(positions);
      else if (type == geometry::bv_type::SPHERE)
        bv = geometry::bounding_volume::create_bounding_sphere
        (geometry::bv_ctor_type::RITTER, positions);
      rc->set_bounding_volume(std::move(bv));
      return rc;
    }

    // setter
    void set_bounding_volume(u_ptr<geometry::bounding_volume>&& bv) { bounding_volume_ = std::move(bv); }

  private:
    u_ptr<geometry::bounding_volume> bounding_volume_;
};

} // namespace game
} // namespace hnll