#pragma once

// hnll
#include <game/component.hpp>
#include <utils/utils.hpp>

//std
#include <functional>

// lib
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>


namespace iscg {

// forward declaration
class drag_manager;

class draggable_component : public hnll::game::component
{
  public:
    static s_ptr<draggable_component> create(const s_ptr<hnll::utils::transform>& transform_sp, float radius = 0.05)
    { return std::make_shared<draggable_component>(transform_sp, radius); }

    draggable_component(const s_ptr<hnll::utils::transform>& transform_sp, float radius = 0.05)
    : transform_sp_(transform_sp), radius_(radius){}

    // TODO : template
    // to identify centroid drag
    void set_drag_callback(std::function<void(bool)> func) { drag_callback_ = func; }
    void execute_drag_callback(bool param) { drag_callback_(param); }

    // getter
    hnll::utils::transform get_transform() { return *transform_sp_; }
    float get_radius() { return radius_; }
    // setter
    void set_transform(hnll::utils::transform& transform) { *transform_sp_ = transform; }

  private:
    s_ptr<hnll::utils::transform> transform_sp_;
    float radius_;
    std::function<void(bool)> drag_callback_ = [](bool){};
};

} // namespace iscg