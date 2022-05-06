#include <hge_components/viewer_component.hpp>

#include <glm/glm.hpp>

// std
#include <cassert>
#include <limits>

namespace hnll {

float ViewerComponent::nearDistance_ = 0.1f;
float ViewerComponent::fovy_ = glm::radians(50.f);

ViewerComponent::ViewerComponent(Transform& transform, HveRenderer& renderer)
  : HgeComponent(), transform_m(transform), hveRenderer_m(renderer)
{
  
}

void ViewerComponent::setOrthographicProjection(
    float left, float right, float top, float bottom, float near, float far) 
{
  projectionMatrix_m = glm::mat4{1.0f};
  projectionMatrix_m[0][0] = 2.f / (right - left);
  projectionMatrix_m[1][1] = 2.f / (bottom - top);
  projectionMatrix_m[2][2] = 1.f / (far - near);
  projectionMatrix_m[3][0] = -(right + left) / (right - left);
  projectionMatrix_m[3][1] = -(bottom + top) / (bottom - top);
  projectionMatrix_m[3][2] = -near / (far - near);
}
 
void ViewerComponent::setPerspectiveProjection(float fovy, float aspect, float near, float far) 
{
  assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
  const float tanHalfFovy = tan(fovy / 2.f);
  projectionMatrix_m = glm::mat4{0.0f};
  projectionMatrix_m[0][0] = 1.f / (aspect * tanHalfFovy);
  projectionMatrix_m[1][1] = 1.f / (tanHalfFovy);
  projectionMatrix_m[2][2] = far / (far - near);
  projectionMatrix_m[2][3] = 1.f;
  projectionMatrix_m[3][2] = -(far * near) / (far - near);
}

void ViewerComponent::setViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up) 
{
  const glm::vec3 w{glm::normalize(direction)};
  const glm::vec3 u{glm::normalize(glm::cross(w, up))};
  const glm::vec3 v{glm::cross(w, u)};

  viewMatrix_m = glm::mat4{1.f};
  viewMatrix_m[0][0] = u.x;
  viewMatrix_m[1][0] = u.y;
  viewMatrix_m[2][0] = u.z;
  viewMatrix_m[0][1] = v.x;
  viewMatrix_m[1][1] = v.y;
  viewMatrix_m[2][1] = v.z;
  viewMatrix_m[0][2] = w.x;
  viewMatrix_m[1][2] = w.y;
  viewMatrix_m[2][2] = w.z;
  viewMatrix_m[3][0] = -glm::dot(u, position);
  viewMatrix_m[3][1] = -glm::dot(v, position);
  viewMatrix_m[3][2] = -glm::dot(w, position);
}

void ViewerComponent::setViewTarget(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) 
{ setViewDirection(position, target - position, up); }

void ViewerComponent::setViewYXZ() 
{
  auto& position = transform_m.translation_m;
  auto& rotation = transform_m.rotation_m;
  const float c3 = glm::cos(rotation.z);
  const float s3 = glm::sin(rotation.z);
  const float c2 = glm::cos(rotation.x);
  const float s2 = glm::sin(rotation.x);
  const float c1 = glm::cos(rotation.y);
  const float s1 = glm::sin(rotation.y);
  const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
  const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
  const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
  viewMatrix_m = glm::mat4{1.f};
  viewMatrix_m[0][0] = u.x;
  viewMatrix_m[1][0] = u.y;
  viewMatrix_m[2][0] = u.z;
  viewMatrix_m[0][1] = v.x;
  viewMatrix_m[1][1] = v.y;
  viewMatrix_m[2][1] = v.z;
  viewMatrix_m[0][2] = w.x;
  viewMatrix_m[1][2] = w.y;
  viewMatrix_m[2][2] = w.z;
  viewMatrix_m[3][0] = -glm::dot(u, position);
  viewMatrix_m[3][1] = -glm::dot(v, position);
  viewMatrix_m[3][2] = -glm::dot(w, position);
}

glm::mat4 ViewerComponent::getInversePerspectiveProjection() const
{
  // TODO : calc this in a safe manner
  return glm::inverse(projectionMatrix_m);
}

glm::mat4 ViewerComponent::getInverseViewYXZ() const
{
  auto position = -transform_m.translation_m;
  auto rotation = -transform_m.rotation_m;
  const float c3 = glm::cos(rotation.z);
  const float s3 = glm::sin(rotation.z);
  const float c2 = glm::cos(rotation.x);
  const float s2 = glm::sin(rotation.x);
  const float c1 = glm::cos(rotation.y);
  const float s1 = glm::sin(rotation.y);
  const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
  const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
  const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
  auto invView = glm::mat4{1.f};
  invView[0][0] = u.x;
  invView[1][0] = u.y;
  invView[2][0] = u.z;
  invView[0][1] = v.x;
  invView[1][1] = v.y;
  invView[2][1] = v.z;
  invView[0][2] = w.x;
  invView[1][2] = w.y;
  invView[2][2] = w.z;
  invView[3][0] = -glm::dot(u, position);
  invView[3][1] = -glm::dot(v, position);
  invView[3][2] = -glm::dot(w, position);
  return invView;
}

// owner's transform should be update by keyMoveComp before this function
void ViewerComponent::updateComponent(float dt)
{ 
  setViewYXZ();
  auto aspect = hveRenderer_m.getAspectRatio();
  setPerspectiveProjection(fovy_, aspect, nearDistance_, 50.f); 
}

} // namesapce hnll