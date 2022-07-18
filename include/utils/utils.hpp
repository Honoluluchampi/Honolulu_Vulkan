#pragma once

// TODO : put this file on a appropriate position
// TODO : create createOneShotCommandPool();

// std
#include <memory>
#include <iostream>

// lib
#include <glm/gtc/matrix_transform.hpp>

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

namespace hnll {
namespace utils {

// 3d transformation
struct transform
{
  glm::vec3 translation{}; // position offset
  glm::vec3 scale{1.f, 1.f, 1.f};
  // y-z-x tait-brian rotation
  glm::vec3 rotation{};

  // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4(); 
  glm::mat4 rotate_mat4();
  // normal = R * S(-1)
  glm::mat3 normal_matrix();
};

static inline glm::vec3 sclXvec(const float scalar, const glm::vec3& vec)
{ return {vec.x * scalar, vec.y * scalar, vec.z * scalar}; }

} // namespace utils
} // namespace hnll