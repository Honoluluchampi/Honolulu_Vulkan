// iscg
#include <bone.hpp>

// hnll
#include <utils/utils.hpp>

namespace iscg {

void bone::update_transform()
{
  auto difference = head_ - tail_;
  // projection to each plane
  // rotation around y axis doesn't matter because a bone is symmetric about y axis
  glm::vec3 direction_in_yz = glm::normalize(glm::vec3(0.f, difference.y, difference.z));
  glm::vec3 direction_in_xy = glm::normalize(glm::vec3(difference.x, difference.y, 0.f));
  // compute rotation around each axes
  auto x_rot = std::acos(glm::dot(glm::vec3(0.f, 1.f, 0.f), direction_in_yz));
  if (!hnll::utils::is_same_handed_system(glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f), direction_in_yz))
    x_rot *= -1;
  auto z_rot = std::acos(glm::dot(direction_in_yz, glm::normalize(difference)));
  if (!hnll::utils::is_same_handed_system(glm::vec3{1.f, 0.f, 0.f}, direction_in_yz, direction_in_yz, difference))
    z_rot *= -1;
  // update rotation
  get_transform_sp()->rotation = {x_rot, 0.f, z_rot};

  // compute length
  auto length = glm::sqrt(glm::dot(difference, difference));
  get_transform_sp()->scale = {length, length, length};
}

} // namespace iscg