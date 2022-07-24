#pragma once

// hnll
#include <game/actor.hpp>
#include <game/components/mesh_component.hpp>
#include <utils/utils.hpp>

// std
#include <memory>

namespace iscg {

constexpr static float DEFAULT_BONE_LENGTH = 2.f;

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
    static s_ptr<bone> create(const s_ptr<bone>& parent = nullptr)
    { auto bn = std::make_shared<bone>(parent); hnll::game::engine::add_actor(bn); return bn; }
    explicit bone(const s_ptr<bone>& parent = nullptr) : hnll::game::actor(), parent_(parent), tail_(get_transform_sp()->translation) {}

    void update_inward_kinematics(const glm::vec3& control_point, const glm::vec3& whole_head_point)
    {
    }

    // Transform which will be modified, rotating transform, base point position
    void point_to(const glm::vec3& target)
    {
      //
    }

    void align_to_parent()
    {
      set_translation(parent_->get_head_translation());
      set_rotation(parent_->get_transform_sp()->rotation);
    }

    // getter
    glm::vec3 get_head_translation() const { return head_; }
    glm::vec3 get_tail_translation() const { return tail_; }
    bool is_root() const { return parent_ == nullptr; }
    bool is_leaf() const { return child_ == nullptr; }
    // setter
    void set_whole_translation(const glm::vec3 &translation) { head_ = translation + (head_ - tail_); tail_ = translation; }
    void set_head_translation(const glm::vec3& translation) { head_ = translation; update_transform(); }
    void set_tail_translation(const glm::vec3& translation) { tail_ = translation; update_transform(); }

  private:
    void update_transform();

    s_ptr<bone> parent_ = nullptr;
    s_ptr<bone> child_ = nullptr;

    glm::vec3 head_ = {0.f, 2.f, 0.f};
    glm::vec3& tail_;
};

} // namespace iscg