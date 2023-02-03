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
namespace geometry { class bounding_volume; enum class bv_type; }

using vec3d = Eigen::Vector3d;

namespace game {

using actor_id           = unsigned int;
using rigid_component_id = unsigned int;

class rigid_component : public component
{
  public:
    static s_ptr<rigid_component> create_with_aabb(actor& owner, const s_ptr<hnll::game::mesh_component>& mesh_component);
    static s_ptr<rigid_component> create_with_b_sphere(actor& owner, const s_ptr<game::mesh_component>& mesh_component);
    static s_ptr<rigid_component> create_from_bounding_volume(actor& owner, u_ptr<geometry::bounding_volume>&& bv);
    static s_ptr<rigid_component> create(actor& owner, const std::vector<vec3d>& positions, geometry::bv_type type);

    explicit rigid_component(actor& owner);
    ~rigid_component() override = default;

    // getter
    [[nodiscard]] const geometry::bounding_volume& get_bounding_volume() const { return *bounding_volume_; }
    [[nodiscard]] const utils::transform&          get_transform_ref()   const { return *transform_sp_; }
    [[nodiscard]] s_ptr<utils::transform>          get_transform()       const { return transform_sp_; }
    [[nodiscard]] rigid_component_id               get_id()              const { return rigid_component_id_; }
    [[nodiscard]] actor_id                         get_owner_id()        const { return owner_id_; }
    [[nodiscard]] double get_mass()                                      const { return mass_; }
    [[nodiscard]] double get_restitution()                               const { return restitution_; }

    // setter
    void set_bounding_volume(u_ptr<geometry::bounding_volume>&& bv) { bounding_volume_ = std::move(bv); }
    void set_transform(const s_ptr<utils::transform>& transform_sp) { transform_sp_ = transform_sp; }
    void set_mass(double mass)                                      { mass_ = mass; }
    void set_restitution(double restitution)                        { restitution_ = restitution; }

  private:
    double mass_ = 1.f;
    double restitution_ = 1.f;
    s_ptr<hnll::utils::transform>    transform_sp_;
    u_ptr<geometry::bounding_volume> bounding_volume_;
    actor_id           owner_id_;
    rigid_component_id rigid_component_id_;
};

} // namespace game
} // namespace hnll