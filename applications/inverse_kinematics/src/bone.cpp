// iscg
#include <bone.hpp>

// hnll
#include <game/engine.hpp>

// hnll
#include <utils/utils.hpp>

namespace iscg {

s_ptr<bone> bone::create(const s_ptr<bone> &parent)
{
  auto bn = std::make_shared<bone>(parent);
  auto bone_model = hnll::game::engine::get_mesh_model_sp("bone");
  auto bone_model_comp = hnll::game::mesh_component::create(bn, std::move(bone_model));

  if (parent != nullptr) {
    parent->set_child(bn);
    bn->align_to_parent();
  }
  hnll::game::engine::add_actor(bn);
  return bn;
}


void bone::rotate_around_point(const glm::mat4 &rotate_mat, const glm::vec3 &base_point)
{
  tail_ -= base_point;
  tail_ = rotate_mat * glm::vec4(tail_, 0.f);
  tail_ += base_point;
  head_ -= base_point;
  head_ = rotate_mat * glm::vec4(head_, 0.f);
  head_ += base_point;
  update_transform();
}

void bone::update_transform()
{
  auto difference = glm::normalize(head_ - tail_);

  glm::vec3 new_z_axis = {0, difference.z, -difference.y};
  double x_rot = std::asin(-difference.z);
         x_rot = std::acos(-difference.y);

//  // projection to each plane
//  // rotation around y axis doesn't matter because a bone is symmetric about y axis
//  glm::vec3 direction_in_yz = glm::normalize(glm::vec3(0.f, difference.y, difference.z));
//  glm::vec3 unit_x{1.f, 0.f, 0.f}, unit_y{0.f, 1.f, 0.f}, unit_z{0.f, 0.f, 1.f};
//  // compute rotation around each axes
//  auto x_rot = std::acos(glm::dot(unit_y, direction_in_yz));
//  if (!hnll::utils::is_same_handed_system(unit_y, unit_z, unit_y, direction_in_yz))
//    x_rot *= -1;
//  auto z_rot = std::acos(glm::dot(direction_in_yz, glm::normalize(difference)));
//  if (!hnll::utils::is_same_handed_system(unit_x, direction_in_yz, direction_in_yz, difference))
//    z_rot *= -1;
  // update rotation
  get_transform_sp()->rotation = {x_rot, 0.f, z_rot};

  // compute length
  auto length = glm::sqrt(glm::dot(difference, difference)) / DEFAULT_BONE_LENGTH;
  get_transform_sp()->scale = {length, length, length};
}

} // namespace iscg