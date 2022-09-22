// hnll
#include <game/actor.hpp>
#include <physics/rigid_component.hpp>

namespace hnll::physics {

void hnll::physics::rigid_component::specific_add_process(hnll::game::actor &owner)
{
  this->transform_sp_ = owner.get_transform_sp();
}

} // hnll::physics