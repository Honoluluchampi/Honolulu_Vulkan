#pragma once

// TODO : put this file on a appropriate position
// TODO : create createOneShotCommandPool();

#include <memory>
#include <iostream>

// lib
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.hpp>

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
  glm::mat4 rotateMat4();
  // normal = R * S(-1)
  glm::mat3 normalMatrix();
};

} // namespace hnll


static inline glm::vec3 sclXvec(const float scalar, const glm::vec3& vec)
{ return {vec.x * scalar, vec.y * scalar, vec.z * scalar}; }

namespace hnll {

glm::mat4 Transform::mat4() 
{
  const float c3 = glm::cos(rotation_m.z), s3 = glm::sin(rotation_m.z), c2 = glm::cos(rotation_m.x), 
    s2 = glm::sin(rotation_m.x), c1 = glm::cos(rotation_m.y), s1 = glm::sin(rotation_m.y);
  return glm::mat4{
      {
          scale_m.x * (c1 * c3 + s1 * s2 * s3),
          scale_m.x * (c2 * s3),
          scale_m.x * (c1 * s2 * s3 - c3 * s1),
          0.0f,
      },
      {
          scale_m.y * (c3 * s1 * s2 - c1 * s3),
          scale_m.y * (c2 * c3),
          scale_m.y * (c1 * c3 * s2 + s1 * s3),
          0.0f,
      },
      {
          scale_m.z * (c2 * s1),
          scale_m.z * (-s2),
          scale_m.z * (c1 * c2),
          0.0f,
      },
      {translation_m.x, translation_m.y, translation_m.z, 1.0f}};
}

glm::mat4 Transform::rotateMat4()
{
  const float c3 = glm::cos(rotation_m.z), s3 = glm::sin(rotation_m.z), c2 = glm::cos(rotation_m.x), 
  s2 = glm::sin(rotation_m.x), c1 = glm::cos(rotation_m.y), s1 = glm::sin(rotation_m.y);
  return glm::mat4{
      {
          scale_m.x * (c1 * c3 + s1 * s2 * s3),
          scale_m.x * (c2 * s3),
          scale_m.x * (c1 * s2 * s3 - c3 * s1),
          0.0f,
      },
      {
          scale_m.y * (c3 * s1 * s2 - c1 * s3),
          scale_m.y * (c2 * c3),
          scale_m.y * (c1 * c3 * s2 + s1 * s3),
          0.0f,
      },
      {
          scale_m.z * (c2 * s1),
          scale_m.z * (-s2),
          scale_m.z * (c1 * c2),
          0.0f,
      },
      {0.f, 0.f, 0.f, 1.0f}};
}

// normal = R * S(-1)
glm::mat3 Transform::normalMatrix()
{
  const float c3 = glm::cos(rotation_m.z), s3 = glm::sin(rotation_m.z), c2 = glm::cos(rotation_m.x), 
    s2 = glm::sin(rotation_m.x), c1 = glm::cos(rotation_m.y), s1 = glm::sin(rotation_m.y);

  const glm::vec3 invScale = 1.0f / scale_m;
  return glm::mat3{
      {
          invScale.x * (c1 * c3 + s1 * s2 * s3),
          invScale.x * (c2 * s3),
          invScale.x * (c1 * s2 * s3 - c3 * s1)
      },
      {
          invScale.y * (c3 * s1 * s2 - c1 * s3),
          invScale.y * (c2 * c3),
          invScale.y * (c1 * c3 * s2 + s1 * s3)
      },
      {
          invScale.z * (c2 * s1),
          invScale.z * (-s2),
          invScale.z * (c1 * c2)
      }
  };
}

}