#pragma once

// iscg
#include <draggable_component.hpp>

// hnll
#include <game/engine.hpp>
#include <game/actor.hpp>
#include <game/components/point_light_component.hpp>
#include <game/components/line_component.hpp>

namespace iscg {

class control_point : public hnll::game::actor
{
  public:
    // const glm::vec3& position, const glm::vec3& color, float radius
    // const std::vector<s_ptr<control_point>>& base_points, const glm::vec3& color, float radius
    template <class... Args>
    static s_ptr<control_point> create(Args... args)
    { auto sp = std::make_shared<control_point>(args...); hnll::game::engine::add_actor(sp); return sp; }
    explicit control_point(const glm::vec3 &position = {0.f, 0.f, 0.f}, const glm::vec3 &color = {.1f, 1.f, 1.f}, float radius = 0.05);

    // for creating centroid (create base points first)
    // takes CoonsSurface::basePoints_
    explicit control_point(const std::vector<s_ptr<control_point>> &base_points, const glm::vec3 &color, float radius);

    // getter
    s_ptr<draggable_component> get_drag_comp_sp() { return drag_comp_sp_; }
    s_ptr<hnll::game::point_light_component> get_light_comp_sp() { return light_comp_sp_; }
    glm::vec3 get_translation() { return this->get_transform_sp()->translation; }

  private:
    s_ptr<draggable_component> drag_comp_sp_;
    s_ptr<hnll::game::point_light_component> light_comp_sp_;
    bool is_centroid_ = false;
};

} // namespace iscg