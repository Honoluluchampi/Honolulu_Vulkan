// hnll
#include <game/component.hpp>

namespace hnll::game {

component::component() { static component_id id = 0; id_ = id++;}

} // namespace hnll::game