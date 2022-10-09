// hnll
#include <game/actor.hpp>
#include <game/components/mesh_component.hpp>
#include <physics/rigid_component.hpp>
#include <geometry/collision_detector.hpp>
#include <geometry/bounding_volume.hpp>

namespace hnll::physics {

rigid_component::rigid_component(u_ptr<geometry::bounding_volume> &&bv)
 : bounding_volume_(std::move(bv)) {}

s_ptr<rigid_component> rigid_component::create_with_aabb(const s_ptr<hnll::game::mesh_component> &mesh_component_sp)
{
  auto mesh_vertices = mesh_component_sp->get_model_sp()->get_vertex_position_list();
  auto bv = geometry::bounding_volume::create_aabb(mesh_vertices);

  // automatically add to the collision_detector (as static member)
  auto rc = std::make_shared<rigid_component>(std::move(bv));
  collision_detector::add_rigid_component(rc);
  return rc;
}

void hnll::physics::rigid_component::specific_add_process(hnll::game::actor &owner)
{
  // use same transform as owner
  this->transform_sp_ = owner.get_transform_sp();
}

} // hnll::physics