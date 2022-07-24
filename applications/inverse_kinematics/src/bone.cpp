// iscg
#include <bone.hpp>

namespace iscg {

void bone::update_transform()
{
  auto difference = head_ - tail_;
  // projection to each planes
  glm::vec3 direction_in_zx = glm::normalize(glm::vec3(difference.x, 0.f, difference.z));
  glm::vec3 direction_in_xy = glm::normalize(glm::vec3(difference.x, difference.y, 0.f));
  // compute rotation around each axes
  auto z_rot = std::acos(glm::dot(direction_in_xy, glm::vec3(1.f, 0.f, 0.f)));
  auto y_rot = std::acos(glm::dot(direction_in_zx, glm::vec3(0.f, 0.f, 1.f)));
  // update rotation
  get_transform_sp()->rotation = glm::vec3(0.f, y_rot, z_rot);

  // compute length
  auto length = glm::sqrt(glm::dot(difference, difference));
  get_transform_sp()->scale = {length, length, length};
}

} // namespace iscg