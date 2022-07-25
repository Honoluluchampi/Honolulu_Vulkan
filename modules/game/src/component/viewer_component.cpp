// hnll
#include <game/components/viewer_component.hpp>

// std
#include <cassert>
#include <limits>

// lib
#include <glm/glm.hpp>

namespace hnll {
namespace game {

float viewer_component::near_distance_ = 0.1f;
float viewer_component::fovy_ = glm::radians(50.f);

viewer_component::viewer_component(hnll::utils::transform& transform, hnll::graphics::renderer& renderer)
  : component(), transform_(transform), renderer_(renderer)
{
  
}

void viewer_component::set_orthographics_projection(
    float left, float right, float top, float bottom, float near, float far) 
{
  projection_matrix_ = glm::mat4{1.0f};
  projection_matrix_[0][0] = 2.f / (right - left);
  projection_matrix_[1][1] = 2.f / (bottom - top);
  projection_matrix_[2][2] = 1.f / (far - near);
  projection_matrix_[3][0] = -(right + left) / (right - left);
  projection_matrix_[3][1] = -(bottom + top) / (bottom - top);
  projection_matrix_[3][2] = -near / (far - near);
}
 
void viewer_component::set_perspective_projection(float fovy, float aspect, float near, float far) 
{
  assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
  const float tan_half_fovy = tan(fovy / 2.f);
  projection_matrix_ = glm::mat4{0.0f};
  projection_matrix_[0][0] = 1.f / (aspect * tan_half_fovy);
  projection_matrix_[1][1] = 1.f / (tan_half_fovy);
  projection_matrix_[2][2] = far / (far - near);
  projection_matrix_[2][3] = 1.f;
  projection_matrix_[3][2] = -(far * near) / (far - near);
}

void viewer_component::set_veiw_direction(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up) 
{
  const glm::vec3 w{glm::normalize(direction)};
  const glm::vec3 u{glm::normalize(glm::cross(w, up))};
  const glm::vec3 v{glm::cross(w, u)};

  view_matrix_ = glm::mat4{1.f};
  view_matrix_[0][0] = u.x;
  view_matrix_[1][0] = u.y;
  view_matrix_[2][0] = u.z;
  view_matrix_[0][1] = v.x;
  view_matrix_[1][1] = v.y;
  view_matrix_[2][1] = v.z;
  view_matrix_[0][2] = w.x;
  view_matrix_[1][2] = w.y;
  view_matrix_[2][2] = w.z;
  view_matrix_[3][0] = -glm::dot(u, position);
  view_matrix_[3][1] = -glm::dot(v, position);
  view_matrix_[3][2] = -glm::dot(w, position);
}

void viewer_component::set_view_target(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) 
{ set_veiw_direction(position, target - position, up); }

void viewer_component::set_veiw_yxz() 
{
  auto& position = transform_.translation;
  auto& rotation = transform_.rotation;
  const float c3 = glm::cos(rotation.z);
  const float s3 = glm::sin(rotation.z);
  const float c2 = glm::cos(rotation.x);
  const float s2 = glm::sin(rotation.x);
  const float c1 = glm::cos(rotation.y);
  const float s1 = glm::sin(rotation.y);
  const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
  const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
  const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
  view_matrix_ = glm::mat4{1.f};
  view_matrix_[0][0] = u.x;
  view_matrix_[1][0] = u.y;
  view_matrix_[2][0] = u.z;
  view_matrix_[0][1] = v.x;
  view_matrix_[1][1] = v.y;
  view_matrix_[2][1] = v.z;
  view_matrix_[0][2] = w.x;
  view_matrix_[1][2] = w.y;
  view_matrix_[2][2] = w.z;
  view_matrix_[3][0] = -glm::dot(u, position);
  view_matrix_[3][1] = -glm::dot(v, position);
  view_matrix_[3][2] = -glm::dot(w, position);
}

glm::mat4 viewer_component::get_inverse_perspective_projection() const
{
  // TODO : calc this in a safe manner
  return glm::inverse(projection_matrix_);
}

glm::mat4 viewer_component::get_inverse_view_yxz() const
{
  auto position = -transform_.translation;
  auto rotation = -transform_.rotation;
  const float c3 = glm::cos(rotation.z);
  const float s3 = glm::sin(rotation.z);
  const float c2 = glm::cos(rotation.x);
  const float s2 = glm::sin(rotation.x);
  const float c1 = glm::cos(rotation.y);
  const float s1 = glm::sin(rotation.y);
  const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
  const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
  const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
  auto inv_view = glm::mat4{1.f};
  inv_view[0][0] = u.x;
  inv_view[1][0] = u.y;
  inv_view[2][0] = u.z;
  inv_view[0][1] = v.x;
  inv_view[1][1] = v.y;
  inv_view[2][1] = v.z;
  inv_view[0][2] = w.x;
  inv_view[1][2] = w.y;
  inv_view[2][2] = w.z;
  inv_view[3][0] = -glm::dot(u, position);
  inv_view[3][1] = -glm::dot(v, position);
  inv_view[3][2] = -glm::dot(w, position);
  return inv_view;
}

// owner's transform should be update by keyMoveComp before this function
void viewer_component::update_component(float dt)
{ 
  set_veiw_yxz();
  auto aspect = renderer_.get_aspect_ratio();
  set_perspective_projection(fovy_, aspect, near_distance_, 50.f); 
}

} // namespace game
} // namesapce hnll