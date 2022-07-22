#include <app.hpp>

namespace iscg {

app::app() : hnll::game::engine("inward kinematics")
{
  // init camera transform
  camera_up_->set_translation(glm::vec3(0.f, 0.f, -10.f));

  // init drag manager
  drag_manager_up_ = std::make_unique<drag_manager>(glfw_window_, *camera_up_);

  // init drag point
  control_point_sp_ = control_point::create(glm::vec3{0.f, 0.f, 0.f}, control_point_color_, control_point_radius_);
  add_point_light_without_owner(control_point_sp_->get_light_comp_sp());
  drag_manager_up_->add_drag_comp(control_point_sp_->get_drag_comp_sp());

  auto light_actor = hnll::game::actor::create();
  auto light_comp = hnll::game::point_light_component::create(10.f, 0.f, {1.f, 1.f, 1.f});
  light_comp->set_translation(glm::vec3(0.f, -10.f, 0.f));
  add_point_light(light_actor, light_comp);
  // root
  bones_.emplace_back(create_bone());
  whole_tail_ = bones_[0];
  // first
  bones_.emplace_back(create_bone(bones_[0]));
  whole_head_ = bones_[1];
}

s_ptr<bone> app::create_bone(s_ptr<bone> parent)
{
  auto bone = bone::create(parent);
  auto bone_model = get_mesh_model_sp("bone");
  auto bone_model_comp = hnll::game::mesh_component::create(bone_model);
  bone->set_renderable_component(bone_model_comp);

  if (parent != nullptr) bone->align_to_parent();

  return bone;
}

void app::update_game(float dt)
{
  drag_manager_up_->update(0.f);

  static bool changed_cache = false;
  if (drag_manager_up_->is_changed()) changed_cache = true;
  if (bind_to_control_point_ && changed_cache) {
    compute_inward_kinematics();
    changed_cache = false;
  }
}

void app::compute_inward_kinematics()
{
  for(auto i = bones_.rbegin(), e = bones_.rend(); i != e; ++i) {
    (*i)->update_inward_kinematics(control_point_sp_->get_translation(), whole_head_->get_head_translation());
  }
}

void app::update_game_gui()
{
  ImGui::Begin("config");

  if (ImGui::Button("create bone")) {
    add_bone();
  }
  if (ImGui::Button("delete bone")) {
    // deleteBone();
  }
  ImGui::Checkbox("bind to control point", &bind_to_control_point_);

  ImGui::End();
}

void app::add_bone()
{
  bones_.emplace_back(create_bone(*(--bones_.end())));
  whole_head_ = *(--bones_.end());
}

void app::delete_bone()
{
  if (bones_.size() == 1)
    std::cout << "cannot delete root!" << std::endl;
  else {
    bones_[bones_.size() - 1]->set_actor_state(hnll::game::actor::state::DEAD);
    bones_.erase(bones_.end() - 1);
  }
}

} // namespace iscg
