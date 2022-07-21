#pragma once

#include <draggable_component.hpp>

// hge
#include <game/actor.hpp>
#include <game/actors/default_camera.hpp>

// lib
#include <GLFW/glfw3.h>

// std
#include <unordered_map>

namespace iscg {

class drag_manager : public hnll::game::actor
{
    using drag_comp_map = std::unordered_map<hnll::game::component_id, s_ptr<draggable_component>>;

  public:
    drag_manager(GLFWwindow* window, hnll::game::default_camera& camera);

    // complete transport
    template <class SP> void add_drag_comp(SP&& drag_comp_sp)
    {
      hnll::game::component_id id = drag_comp_sp->get_id();
      drag_comp_map_.emplace(id, std::forward<SP>(drag_comp_sp));
    }
    void remove_drag_comp(hnll::game::component_id id) { drag_comp_map_.erase(id); }

    inline bool is_changed() const { return is_changed_; }

  private:
    void update_actor(float dt) override;

    void calc_cursor_projection_intersect();
    glm::vec2 calc_raw_click_point();
    glm::vec3 calc_world_click_point(const glm::vec2& click_point);

    // controll mouse-point binding
    void set_glfw_mouse_button_callback();

    drag_comp_map drag_comp_map_;

    GLFWwindow* window_;
    static bool is_dragging_;
    static bool is_bound_;
    bool is_changed_ = false;
    // -1 : no comp is bound
    hnll::game::component_id bound_comp_id_ = -1;
    // for calculation of new comp pos
    float camera_originated_comp_z_;
    hnll::game::default_camera& camera_;
};

} // namespace iscg