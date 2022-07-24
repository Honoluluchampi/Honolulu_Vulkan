// hnll
#include <utils/utils.hpp>

namespace hnll {
namespace utils {

glm::mat4 transform::mat4()
{
  const float c3 = glm::cos(rotation.z), s3 = glm::sin(rotation.z), c2 = glm::cos(rotation.x),
    s2 = glm::sin(rotation.x), c1 = glm::cos(rotation.y), s1 = glm::sin(rotation.y);
  return glm::mat4{
      {
          scale.x * (c1 * c3 + s1 * s2 * s3),
          scale.x * (c2 * s3),
          scale.x * (c1 * s2 * s3 - c3 * s1),
          0.0f,
      },
      {
          scale.y * (c3 * s1 * s2 - c1 * s3),
          scale.y * (c2 * c3),
          scale.y * (c1 * c3 * s2 + s1 * s3),
          0.0f,
      },
      {
          scale.z * (c2 * s1),
          scale.z * (-s2),
          scale.z * (c1 * c2),
          0.0f,
      },
      {translation.x, translation.y, translation.z, 1.0f}};
}

glm::mat4 transform::rotate_mat4()
{
  const float c3 = glm::cos(rotation.z), s3 = glm::sin(rotation.z), c2 = glm::cos(rotation.x),
  s2 = glm::sin(rotation.x), c1 = glm::cos(rotation.y), s1 = glm::sin(rotation.y);
  return glm::mat4{
      {
          scale.x * (c1 * c3 + s1 * s2 * s3),
          scale.x * (c2 * s3),
          scale.x * (c1 * s2 * s3 - c3 * s1),
          0.0f,
      },
      {
          scale.y * (c3 * s1 * s2 - c1 * s3),
          scale.y * (c2 * c3),
          scale.y * (c1 * c3 * s2 + s1 * s3),
          0.0f,
      },
      {
          scale.z * (c2 * s1),
          scale.z * (-s2),
          scale.z * (c1 * c2),
          0.0f,
      },
      {0.f, 0.f, 0.f, 1.0f}};
}

glm::mat3 transform::rotate_mat3()
{
  const float c3 = glm::cos(rotation.z), s3 = glm::sin(rotation.z), c2 = glm::cos(rotation.x),
  s2 = glm::sin(rotation.x), c1 = glm::cos(rotation.y), s1 = glm::sin(rotation.y);
  return glm::mat3{
    {
      scale.x * (c1 * c3 + s1 * s2 * s3),
      scale.x * (c2 * s3),
      scale.x * (c1 * s2 * s3 - c3 * s1)
      },
      {
      scale.y * (c3 * s1 * s2 - c1 * s3),
      scale.y * (c2 * c3),
      scale.y * (c1 * c3 * s2 + s1 * s3)
      },
      {
      scale.z * (c2 * s1),
      scale.z * (-s2),
      scale.z * (c1 * c2)
      }};
}

// normal = R * S(-1)
glm::mat3 transform::normal_matrix()
{
  const float c3 = glm::cos(rotation.z), s3 = glm::sin(rotation.z), c2 = glm::cos(rotation.x),
    s2 = glm::sin(rotation.x), c1 = glm::cos(rotation.y), s1 = glm::sin(rotation.y);

  const glm::vec3 inv_scale = 1.0f / scale;
  return glm::mat3{
      {
          inv_scale.x * (c1 * c3 + s1 * s2 * s3),
          inv_scale.x * (c2 * s3),
          inv_scale.x * (c1 * s2 * s3 - c3 * s1)
      },
      {
          inv_scale.y * (c3 * s1 * s2 - c1 * s3),
          inv_scale.y * (c2 * c3),
          inv_scale.y * (c1 * c3 * s2 + s1 * s3)
      },
      {
          inv_scale.z * (c2 * s1),
          inv_scale.z * (-s2),
          inv_scale.z * (c1 * c2)
      }
  };
}

} // namespace utils
} // namespace hnll