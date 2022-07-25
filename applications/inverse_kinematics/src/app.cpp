#include <app.hpp>

namespace iscg {

app::app() : hnll::game::engine("inverse kinematics")
{
  // init camera transform
  camera_up_->set_translation(glm::vec3(0.f, 0.f, -10.f));

  // init drag manager
  drag_manager_up_ = std::make_unique<drag_manager>(glfw_window_, *camera_up_);

  // init drag point
  control_point_sp_ = control_point::create(glm::vec3{0.f, 0.f, 0.f}, control_point_color_, control_point_radius_);
  add_point_light_without_owner(control_point_sp_->get_light_comp_sp());
  drag_manager_up_->add_drag_comp(control_point_sp_->get_drag_comp_sp());

  control_point_sp_->set_translation({- 10.f * std::sqrt(2)/2, 10/2, -10/2});

  // for illumination
  auto light_actor = hnll::game::actor::create();
  auto light_comp = hnll::game::point_light_component::create(light_actor, 30.f, 0.f, {1.f, 1.f, 1.f});
  light_comp->set_translation(glm::vec3(0.f, -10.f, 0.f));
  add_point_light(light_actor, light_comp);

  // tail
  bones_.emplace_back(bone::create());
  whole_tail_ = bones_[0];
  // head
  bones_.emplace_back(bone::create(bones_[0]));
  whole_head_ = bones_[1];
}

void app::update_game(float dt)
{
  drag_manager_up_->update(0.f);

  static bool changed_cache = false;
  if (drag_manager_up_->is_changed()) changed_cache = true;
  if (bind_to_control_point_ && changed_cache) {
    compute_inverse_kinematics();
    changed_cache = false;
  }
}

bool roughly_equal(const glm::vec3& a, const glm::vec3& b, double epsilon = 0.001)
{
  if (std::abs(a.x - b.x) > epsilon) return false;
  if (std::abs(a.y - b.y) > epsilon) return false;
  if (std::abs(a.z - b.z) > epsilon) return false;
  return true;
}

void app::compute_inverse_kinematics()
{
  // update the bone sequence from the head to the tail
  for(auto bone = bones_.rbegin(), end = bones_.rend(); bone != end; ++bone) {
    // pointing vector from the current bone to the control point
    auto to_control_point = control_point_sp_->get_translation() - (*bone)->get_tail_translation();
    // pointing vector from the current bone to the head
    auto to_head = whole_head_->get_head_translation() - (*bone)->get_tail_translation();
    // base point of rotation
    auto rotation_base = (*bone)->get_tail_translation();
    // axis and angle of rotation from the head to the control point
    auto rotate_axis = glm::normalize(glm::cross(to_head, to_control_point));
    auto rotate_angle = std::acos(glm::dot(glm::normalize(to_head), glm::normalize(to_control_point)));
    // prevent matrix elements' explosion
    if (rotate_angle < 0.03f) continue;
    auto rotate_matrix = glm::rotate(glm::mat4{1.f}, rotate_angle, rotate_axis);
    // rotate bones
    auto current_bone = (*bone);
    current_bone->rotate_around_point(rotate_matrix, rotation_base);
    while (current_bone->has_child()) {
      current_bone = current_bone->get_child_sp();
      current_bone->rotate_around_point(rotate_matrix, rotation_base);
    }
  }

  // bone position assertion
  // a tail should have same translation as its parent head
  for(const auto& bone : bones_) {
    if (bone->has_parent())
      assert(roughly_equal(bone->get_tail_translation(), bone->get_parent_sp()->get_head_translation()));
    else
      assert(roughly_equal(bone->get_tail_translation(), {0.f, 0.f, 0.f}));
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
  auto new_bone = bone::create(whole_head_);
  bones_.emplace_back(new_bone);
  whole_head_ = std::move(new_bone);
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
