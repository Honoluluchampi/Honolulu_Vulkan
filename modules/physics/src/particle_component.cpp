// hnll
#include <physics/particle_component.hpp>
#include <game/actor.hpp>

using hnll::utils::transform;

namespace hnll {
namespace physics {

s_ptr<particle_component> particle_component::create_particle_component(game::actor &actor)
{
  auto particle = std::make_shared<particle_component>();
  actor.get_transform_sp();
}

} // namespace hnll
} // namespace physics