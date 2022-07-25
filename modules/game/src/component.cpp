// hnll
#include <game/component.hpp>

namespace hnll {
namespace game {

component::component() { static component_id id = 0; id_ = id++;}

} // namespace game
} // namespace hnll