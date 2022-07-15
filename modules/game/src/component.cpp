// hnll
#include <game/component.hpp>

namespace hnll {
namespace game {

component::component() { static id id = 0; id_ = id++;}

} // namespace game
} // namespace hnll