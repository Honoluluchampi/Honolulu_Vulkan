#pragma once

#include <memory>

// lib
#include <glm/gtc/matrix_transform.hpp>

namespace hnll {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

// 3d transformation
struct Transform
{
  glm::vec3 translation_m{}; // position offset
  glm::vec3 scale_m{1.f, 1.f, 1.f};
  // y-z-x tait-brian rotation
  glm::vec3 rotation_m{};

  // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4();
  glm::mat3 normalMatrix();
};

} // namespace hnll