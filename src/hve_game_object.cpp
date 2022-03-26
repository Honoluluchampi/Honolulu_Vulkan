#include <hve_game_object.hpp>

namespace hve {

glm::mat4 TransformComponent::mat4() 
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

// normal = R * S(-1)
glm::mat3 TransformComponent::normalMatrix()
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