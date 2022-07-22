#pragma once

// hnll
#include <game/actor.hpp>
#include <game/components/mesh_component.hpp>
#include <utils/utils.hpp>

// std
#include <memory>

namespace iscg {

constexpr static float BONE_LENGTH = 2.f;

template<class T> using s_ptr = std::shared_ptr<T>;
template<class T> using u_ptr = std::unique_ptr<T>;

inline float abs(glm::vec2& vec) { return std::sqrt(vec.x * vec.x + vec.y * vec.y); }
inline float dot(glm::vec2& a, glm::vec2& b) { return a.x * b.x + a.y * b.y; }
inline float abs(glm::vec3& vec) { return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z); }

inline glm::vec3 rotateDifference(const glm::vec3& a, const glm::vec3& b)
{
  glm::vec2 axy = {a.x, a.y}, ayz = {a.y, a.z}, azx = {a.z, a.x};
  glm::vec2 bxy = {b.x, b.y}, byz = {b.y, b.z}, bzx = {b.z, b.x};

  float x = std::acos((ayz.x * byz.y - ayz.y * byz.x >= 0.f) * dot(ayz, byz) / (abs(ayz) * abs(byz))) * 180 / M_PI;
  float y = std::acos((azx.x * bzx.y - azx.y * bzx.x >= 0.f) * dot(azx, bzx) / (abs(azx) * abs(bzx))) * 180 / M_PI;
  float z = std::acos((axy.x * bxy.y - axy.y * bxy.x >= 0.f) * dot(axy, bxy) / (abs(axy) * abs(bxy))) * 180 / M_PI;

  if (!(x >= 0 && x < 360)) x = 0;
  if (!(y >= 0 && y < 360)) y = 0;
  if (!(z >= 0 && z < 360)) z = 0;
  return { x, y, z };
}

class bone : public hnll::game::actor
{
  public:
    static s_ptr<bone> create(const s_ptr<bone>& parent = nullptr) { return std::make_shared<bone>(parent); }
    explicit bone(const s_ptr<bone>& parent = nullptr) : hnll::game::actor(), parent_(parent) {}

    void update_inward_kinematics(const glm::vec3& control_point, const glm::vec3& whole_head_point)
    {
//      auto diff_to_control_point = control_point - this->get_tail_translation();
//      auto diff_to_head_point = whole_head_point - this->get_tail_translation();
//
//      hnll::utils::transform rotate_transform{};
//      rotate_transform.rotation += rotateDifference(diffToCp, diffToHp);
//      // float dot = diffToCp.x * diffToHp.x + diffToCp.y * diffToHp.y + diffToCp.z * diffToHp.z;
//      // float degree = dot / (abs(diffToHp) * abs(diffToCp));
//      // glm::mat4 rotation = glm::rotate(glm::mat4{}, degree, glm::cross(diffToHp, diffToCp));
//
//      // rotate myself
//      rotateAroundPoint(this->transform(), rotateTransform, this->tail());
//      // rotate childlen
//      auto& child = this->child_;
//      while (child != nullptr) {
//        child->rotate_around_point(child->transform(), rotateTransform, this->tail());
//        child = child->child_;
//      }
    }

    // Transform which will be modified, rotating transform, base point position
    void rotate_around_point(hnll::utils::transform& target_transform, hnll::utils::transform& rotate_transform, const glm::vec3& base_point_translation)
    {
      target_transform.translation -= base_point_translation;

      target_transform.translation = rotate_transform.rotate_mat4() * glm::vec4(target_transform.translation, 1.0f);
      target_transform.rotation += rotate_transform.rotation;

      target_transform.translation += base_point_translation;
    }

    // getter
    glm::vec3 get_head_translation()
    {
      auto tf = *get_transform_sp();
      glm::vec3 diff = BONE_LENGTH * tf.rotate_mat4() * glm::vec4(0.f, 1.f, 0.f, 0.f);
      return tf.translation + diff;
    }
    glm::vec3 get_tail_translation() { return get_transform_sp()->translation; }
    bool is_root() const { return parent_ == nullptr; }

    void align_to_parent()
    {
      set_translation(parent_->get_head_translation());
      set_rotation(parent_->get_transform_sp()->rotation);
    }

  private:
    s_ptr<bone> parent_ = nullptr;
    s_ptr<bone> child_ = nullptr;

    float length_ = 3.0f;
    // take care to only accept rigid transform
    s_ptr<hnll::utils::transform> transform_sp_{};
    s_ptr<hnll::game::mesh_component> bone_model_comp_sp_;
};

} // namespace iscg