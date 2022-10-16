// hnll
#include <game/components/rigid_component.hpp>
#include <game/components/mesh_component.hpp>
#include <geometry/bounding_volume.hpp>
#include <physics/collision_detector.hpp>

namespace hnll::game {

s_ptr<rigid_component> rigid_component::create_with_aabb(s_ptr<actor>& owner, const s_ptr<hnll::game::mesh_component>& mesh_component)
{
  auto mesh_vertices = mesh_component->get_model_sp()->get_vertex_position_list();
  auto bv = geometry::bounding_volume::create_aabb(mesh_vertices);

  // automatically add to the intersection (as static member)
  auto rc = std::make_shared<rigid_component>(owner);
  rc->set_bounding_volume(std::move(bv));
  physics::collision_detector::add_rigid_component(rc);
  return rc;
}

s_ptr<rigid_component> create_with_b_sphere(s_ptr<actor>& owner, const s_ptr<game::mesh_component>& mesh_component)
{

}

s_ptr<rigid_component> rigid_component::create_from_bounding_volume(s_ptr<actor>& owner, u_ptr<geometry::bounding_volume>&& bv)
{
  auto rc = std::make_shared<rigid_component>(owner);
  rc->set_bounding_volume(std::move(bv));
  return rc;
}

static s_ptr<rigid_component> create(s_ptr<actor>& owner, const std::vector<vec3>& positions, geometry::bv_type type)
{
  auto rc = std::make_shared<rigid_component>(owner);
  u_ptr<geometry::bounding_volume> bv;
  if (type == geometry::bv_type::AABB)
    bv = geometry::bounding_volume::create_aabb(positions);
  else if (type == geometry::bv_type::SPHERE)
    bv = geometry::bounding_volume::create_bounding_sphere
    (geometry::bv_ctor_type::RITTER, positions);
  rc->set_bounding_volume(std::move(bv));
  return rc;
}

rigid_component::rigid_component(s_ptr<actor>& owner) : component()
{
  this->owner_ = owner;
  // use same transform as owner's
  this->transform_sp_ = owner->get_transform_sp();
}

void rigid_component::re_update_owner(physics::collision_info&& info)
{
  // TODO : impl
}

} // namespace hnll::game