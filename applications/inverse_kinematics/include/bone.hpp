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

class bone : public hnll::game::actor
{
  public:
    static s_ptr<bone> create(const s_ptr<bone>& parent = nullptr);
    explicit bone(const s_ptr<bone>& parent = nullptr) : hnll::game::actor(), parent_(parent), tail_(get_transform_sp()->translation) {}

    void align_to_parent()
    {
      set_whole_translation(parent_->get_head_translation());
      update_transform();
    }

    void rotate_around_point(const glm::mat4& rotate_mat, const glm::vec3& base_point);

    // getter
    glm::vec3 get_head_translation() const { return head_; }
    glm::vec3 get_tail_translation() const { return tail_; }
    s_ptr<bone> get_child_sp() const { return child_; }
    bool has_parent() const { return parent_ != nullptr; }
    bool has_child() const { return child_ != nullptr; }
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