#pragma once

// hge
#include <hge_components/hge_renderable_component.hpp>

// lib
#include <glm/glm.hpp>

namespace hnll {

class LineComponent : public HgeRenderableComponent
{
  public:
    LineComponent(HgeActor::actorId id, glm::vec3& head, glm::vec3& tail, glm::vec3 color, float radius) 
    : HgeRenderableComponent(id, RenderType::LINE), head_(head), tail_(tail), color_(color), radius_(radius)
    {}
    ~LineComponent(){}
    // setter
    void setHead(const glm::vec3& head) { head_ = head; }
    void setTail(const glm::vec3& tail) { tail_ = tail; }
    // getter
    glm::vec3& getHead() { return head_; }
    glm::vec3& getTail() { return tail_; }
    glm::vec3& getColor() { return color_; }
    float getRadius() const { return radius_; }

  private:
    // bind to something
    glm::vec3& head_, &tail_;
    glm::vec3 color_{1.f};
    float radius_ = 1.f;
};

} // namespace hnll