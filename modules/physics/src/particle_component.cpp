// hnll
#include <physics/particle_component.hpp>
#include <game/actor.hpp>

using hnll::utils::transform;

namespace hnll {
namespace physics {

particle_component::particle_component(s_ptr<hnll::utils::transform>&& transform_sp)
 : transform_sp_(std::move(transform_sp)), position_ref_(transform_sp->translation)
{}

s_ptr<particle_component> particle_component::create(game::actor &actor)
{
  auto particle = std::make_shared<particle_component>(actor.get_transform_sp());
  return particle;
}

} // namespace physics
} // namespace hnll