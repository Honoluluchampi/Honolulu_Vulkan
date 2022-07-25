// iscg
#include <bone.hpp>

// hnll
#include <game/engine.hpp>
#include <utils/utils.hpp>

namespace iscg {

s_ptr<bone> bone::create(const s_ptr<bone> &parent)
{
  auto bn = std::make_shared<bone>(parent);
  auto bone_model = hnll::game::engine::get_mesh_model_sp("bone");
  auto bone_model_comp = hnll::game::mesh_component::create(bn, std::move(bone_model));

  if (parent != nullptr) {
    parent->set_child_sp(bn);
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

  float epsilon = 0.001;
  glm::vec3 new_z_axis = {0.f, 0.f, 1.f};
  glm::vec3 new_bone_direction = {0.f, 1.f, 0.f};
  // compute rotation angle around x axis
  if (std::abs(difference.z) > epsilon) {
    new_z_axis = glm::normalize(glm::vec3{0, 1, -difference.y / difference.z});
    float x_rot = -std::asin(new_z_axis.y);
    if (std::abs(std::cos(x_rot) - new_z_axis.z) > epsilon) {
      x_rot = M_PI - x_rot;
    }
    // rotate around x axis
    glm::mat4 x_rot_mat = glm::rotate(glm::mat4{1.f}, x_rot, glm::vec3{1.f, 0.f, 0.f});
    new_bone_direction = x_rot_mat * glm::vec4{0.f, 1.f, 0.f, 0.f};

    get_transform_sp()->rotation.x = x_rot;
  }
  // compute rotation angle around new z axis
  float cos_z_rot = glm::dot(glm::normalize(new_bone_direction), glm::normalize(difference));
  float z_rot = std::acos(cos_z_rot);
  if (glm::dot(glm::normalize(new_z_axis), glm::normalize(glm::cross(new_bone_direction, difference))) < 0) {
    z_rot *= -1;
  }
  get_transform_sp()->rotation.z = z_rot;

  // compute length
  auto length = glm::sqrt(glm::dot(head_ - tail_, head_ - tail_)) / DEFAULT_BONE_LENGTH;
  get_transform_sp()->scale = {length, length, length};
}

} // namespace iscg