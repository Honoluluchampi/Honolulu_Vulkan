#pragma once

// hnll
#include <game/component.hpp>
#include <utils/utils.hpp>
#include <eigen3/Eigen/Dense>

// std
#include <vector>
#include <memory>

namespace hnll {

// forward declaration
namespace game     { class mesh_component; }
namespace physics  { struct collision_info; }
namespace geometry {
  class bounding_volume;
  enum class bv_type;
}

using vec3 = Eigen::Vector3d;

namespace game {

class rigid_component : public component
{
  public:
    static s_ptr<rigid_component> create_with_aabb(s_ptr<actor>& owner, const s_ptr<hnll::game::mesh_component>& mesh_component);
    static s_ptr<rigid_component> create_with_b_sphere(s_ptr<actor>& owner, const s_ptr<game::mesh_component>& mesh_component);
    static s_ptr<rigid_component> create_from_bounding_volume(s_ptr<actor>& owner, u_ptr<geometry::bounding_volume>&& bv);
    static s_ptr<rigid_component> create(s_ptr<actor>& owner, const std::vector<vec3>& positions, geometry::bv_type type);

    explicit rigid_component(s_ptr<actor>& owner);
    ~rigid_component() override = default;

    void re_update_owner(physics::collision_info&& info);

    // getter
    [[nodiscard]] const geometry::bounding_volume& get_bounding_volume() const { return *bounding_volume_; }
    [[nodiscard]] const utils::transform&          get_transform_ref()   const { return *transform_sp_; }
    [[nodiscard]] s_ptr<utils::transform>          get_transform()       const { return transform_sp_; }

    // setter
    void set_bounding_volume(u_ptr<geometry::bounding_volume>&& bv) { bounding_volume_ = std::move(bv); }
    void set_transform(const s_ptr<utils::transform>& transform_sp) { transform_sp_ = transform_sp; }

  private:
    s_ptr<actor> owner_;
    s_ptr<hnll::utils::transform>    transform_sp_;
    u_ptr<geometry::bounding_volume> bounding_volume_;
};

} // namespace game
} // namespace hnll