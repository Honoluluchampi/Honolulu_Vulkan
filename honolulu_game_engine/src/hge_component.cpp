#include <hge_component.hpp>

namespace hnll {

HgeComponent::HgeComponent()
{
  static id_t id = 0;
  id_m = id++;
}
} // namespace hnll