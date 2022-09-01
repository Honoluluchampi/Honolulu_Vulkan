// iscg
#include <drag_manager.hpp>

// hnll
#include <game/engine.hpp>

// lib
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <eigen3/Eigen/Dense>

namespace iscg {

bool drag_manager::is_dragging_ = false;
bool drag_manager::is_bound_ = false;

drag_manager::drag_manager(GLFWwindow* window, hnll::game::default_camera& camera)
    : window_(window), camera_(camera)
{ set_glfw_mouse_button_callback(); }

void drag_manager::set_glfw_mouse_button_callback()
{
  auto func = [](GLFWwindow* window, int button, int action, int mods)
  {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
      is_dragging_ = true;
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      is_dragging_ = false;
      is_bound_ = false;
    }
  };

  hnll::game::engine::add_glfw_mouse_button_callback(std::make_unique<std::function<void(GLFWwindow*, int, int ,int)>>(func));
}

void drag_manager::update_actor(float dt)
{
  is_changed_ = false;
  if (!is_dragging_) return ;

  calc_cursor_projection_intersect();
}

glm::vec2 drag_manager::calc_raw_click_point()
{
  int window_x, window_y;
  glfwGetWindowSize(window_, &window_x, &window_y);
  double click_x, click_y;
  glfwGetCursorPos(window_, &click_x, &click_y);

  // center origin
  click_x -= window_x / 2.f;
  click_x /= window_x / 2.f;
  click_y -= window_y / 2.f;
  click_y /= window_y / 2.f;

  return {click_x, click_y};
}

// takes click point, drag comp pos, projected radius of drag comp
float on_the_line_judge(glm::vec2& a, glm::vec2& b, float buffer)
{
  if (glm::distance(a, b) > buffer) return -1;
  // the larger the buffer is, the closer the comp is to the camera
  return buffer;
}

void drag_manager::calc_cursor_projection_intersect()
{
  glm::vec2 click_point = calc_raw_click_point();

  if (!is_bound_) {
    float max_priority = -1;
    // if cursor is dragging something
    for (const auto& kv : drag_comp_map_) {
      const auto& comp = kv.second;
      // calc projected (virtual) sphere
      glm::vec3 camera_originate_comp_translation =
          camera_.get_viewer_component_sp()->get_projection() *
          camera_.get_viewer_component_sp()->get_view() *
          glm::vec4(comp->get_transform().translation, 1.0f);

      glm::vec2 comp_screen_pos = {camera_originate_comp_translation.x / camera_originate_comp_translation.z, camera_originate_comp_translation.y / camera_originate_comp_translation.z};
      // TODO : calc proper buffer coefficient
      float buffer = comp->get_radius() / camera_originate_comp_translation.z * 2;

      // relaxation
      auto priority = on_the_line_judge(click_point, comp_screen_pos, buffer);
      if (priority == -1) continue;
      if (max_priority == -1 || priority > max_priority) {
        max_priority = priority;
        bound_comp_id_ = comp->get_id();
        camera_originated_comp_z_ = camera_originate_comp_translation.z;
      }
    }
    if (max_priority != -1) is_bound_ = true;
  }
  if (is_bound_) {
    // TODO : calc new position correctly
    auto view = camera_.get_viewer_component_sp();
    auto z = drag_comp_map_[bound_comp_id_]->get_transform().translation.z;
    glm::vec3  camera_comp_point = calc_world_click_point(click_point);
    auto projection_vector = camera_comp_point - camera_.get_transform().translation;
    projection_vector *= camera_originated_comp_z_ / hnll::game::viewer_component::get_near_distance();
    auto new_position = camera_.get_transform().translation + projection_vector;

    if (new_position != drag_comp_map_[bound_comp_id_]->get_translation()) {
      drag_comp_map_[bound_comp_id_]->set_translation(camera_.get_transform().translation + projection_vector);
      is_changed_ = true;
    }
  }
}

glm::mat4 eigen_to_glm(const Eigen::Matrix4d& eigen_mat)
{
  glm::mat4 glm_mat;
  glm_mat[0][0] = eigen_mat(0,0);
  glm_mat[0][1] = eigen_mat(1,0);
  glm_mat[0][2] = eigen_mat(2,0);
  glm_mat[0][3] = eigen_mat(3,0);
  glm_mat[1][0] = eigen_mat(0,1);
  glm_mat[1][1] = eigen_mat(1,1);
  glm_mat[1][2] = eigen_mat(2,1);
  glm_mat[1][3] = eigen_mat(3,1);
  glm_mat[2][0] = eigen_mat(0,2);
  glm_mat[2][1] = eigen_mat(1,2);
  glm_mat[2][2] = eigen_mat(2,2);
  glm_mat[2][3] = eigen_mat(3,2);
  glm_mat[3][0] = eigen_mat(0,3);
  glm_mat[3][1] = eigen_mat(1,3);
  glm_mat[3][2] = eigen_mat(2,3);
  glm_mat[3][3] = eigen_mat(3,3);
  return glm_mat;
}

glm::vec3 drag_manager::calc_world_click_point(const glm::vec2 &click_point)
{
  auto near = hnll::game::viewer_component::get_near_distance();
  auto world_width = near * std::tan(hnll::game::viewer_component::get_fovy() / 1.58f);
  int w, h; glfwGetWindowSize(window_, &w, &h);
  auto world_height = world_width * ((float)h / (float)w);

  return eigen_to_glm(camera_.get_transform().mat4()) * glm::vec4(click_point.x * world_width, click_point.y * world_height, near, 1);
}

} // namespace iscg
