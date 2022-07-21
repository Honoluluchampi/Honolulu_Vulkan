#pragma once

// iscg
#include <bone.hpp>
#include <drag_manager.hpp>
#include <control_point.hpp>

// hge
#include <game/engine.hpp>

namespace iscg {

  class app : public hnll::game::engine
{
  public:
    app();
    ~app();
  private:
    s_ptr<bone> create_bone(s_ptr<bone> parent = nullptr);
    void update_game(float dt) override;
    void update_game_gui() override;

    void add_bone();
    void delete_bone();

    void compute_inward_kinematics();

    std::vector<s_ptr<bone>> bones_;
    s_ptr<bone> whole_head_;
    s_ptr<bone> whole_tail_;

    u_ptr<drag_manager> drag_manager_sp_;
    s_ptr<control_point> control_point_sp_;
    glm::vec3 control_point_color_ = {.1f, .9f, .1f};
    float control_point_radius_ = 0.15f;
    bool bind_to_control_point_ = false;
};

} // namespace iscg