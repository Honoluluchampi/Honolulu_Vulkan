#pragma once

// hnll
#include <game/components/renderable_component.hpp>

// lib
#include <glm/glm.hpp>

namespace hnll {
namespace game {

class line_component : public renderable_component
{
  public:
    // TODO : line_component::create()
    template <Actor A>
    line_component(s_ptr<A>& owner, glm::vec3& head, glm::vec3& tail, glm::vec3 color, float radius)
    : renderable_component(owner, render_type::LINE), head_(head), tail_(tail), color_(color), radius_(radius) {}
    ~line_component(){}

    // setter
    void set_head(const glm::vec3& head) { head_ = head; }
    void set_tail(const glm::vec3& tail) { tail_ = tail; }
    // getter
    glm::vec3& get_head() { return head_; }
    glm::vec3& get_tail() { return tail_; }
    glm::vec3& get_color() { return color_; }
    float get_radius() const { return radius_; }

  private:
    // bind to something
    glm::vec3& head_, &tail_;
    glm::vec3 color_{1.f};
    float radius_ = 1.f;
};

} // namespace game
} // namespace hnll