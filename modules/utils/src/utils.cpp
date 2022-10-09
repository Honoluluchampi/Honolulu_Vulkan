// hnll
#include <utils/utils.hpp>

namespace hnll::utils {

Eigen::Matrix4d transform::mat4() const
{
  const float c3 = std::cos(rotation.z), s3 = std::sin(rotation.z), c2 = std::cos(rotation.x),
    s2 = std::sin(rotation.x), c1 = std::cos(rotation.y), s1 = std::sin(rotation.y);

  Eigen::Matrix4d ret;
  ret << scale.x * (c1 * c3 + s1 * s2 * s3), scale.y * (c3 * s1 * s2 - c1 * s3), scale.z * (c2 * s1), translation.x,
         scale.x * (c2 * s3),                scale.y * (c2 * c3),                scale.z * (-s2),     translation.y,
         scale.x * (c1 * s2 * s3 - c3 * s1), scale.y * (c1 * c3 * s2 + s1 * s3), scale.z * (c1 * c2), translation.z,
         0.f,                                0.f,                                0.f,                 1.f;
  return ret;
}

Eigen::Matrix4d transform::rotate_mat4() const
{
  const float c3 = std::cos(rotation.z), s3 = std::sin(rotation.z), c2 = std::cos(rotation.x),
  s2 = std::sin(rotation.x), c1 = std::cos(rotation.y), s1 = std::sin(rotation.y);

  Eigen::Matrix4d ret;
  ret << scale.x * (c1 * c3 + s1 * s2 * s3), scale.y * (c3 * s1 * s2 - c1 * s3), scale.z * (c2 * s1), 0.f,
         scale.x * (c2 * s3),                scale.y * (c2 * c3),                scale.z * (-s2),     0.f,
         scale.x * (c1 * s2 * s3 - c3 * s1), scale.y * (c1 * c3 * s2 + s1 * s3), scale.z * (c1 * c2), 0.f,
         0.f,                                0.f,                                0.f,                 1.f;
  return ret;
}

Eigen::Matrix3d transform::rotate_mat3() const
{
  const float c3 = std::cos(rotation.z), s3 = std::sin(rotation.z), c2 = std::cos(rotation.x),
  s2 = std::sin(rotation.x), c1 = std::cos(rotation.y), s1 = std::sin(rotation.y);

  Eigen::Matrix3d ret;
  ret << scale.x * (c1 * c3 + s1 * s2 * s3), scale.y * (c3 * s1 * s2 - c1 * s3), scale.z * (c2 * s1),
         scale.x * (c2 * s3),                scale.y * (c2 * c3),                scale.z * (-s2),
         scale.x * (c1 * s2 * s3 - c3 * s1), scale.y * (c1 * c3 * s2 + s1 * s3), scale.z * (c1 * c2);
  return ret;
}

// normal = R * S(-1)
Eigen::Matrix4d transform::normal_matrix() const
{
  const float c3 = std::cos(rotation.z), s3 = std::sin(rotation.z), c2 = std::cos(rotation.x),
    s2 = std::sin(rotation.x), c1 = std::cos(rotation.y), s1 = std::sin(rotation.y);
  const Eigen::Vector3d inv_scale = {1.f / scale.x, 1.f / scale.y, 1.f / scale.z};

  Eigen::Matrix4d ret;
  ret << inv_scale.x() * (c1 * c3 + s1 * s2 * s3), inv_scale.y() * (c3 * s1 * s2 - c1 * s3), inv_scale.z() * (c2 * s1), 0.f,
         inv_scale.x() * (c2 * s3),                inv_scale.y() * (c2 * c3),                inv_scale.z() * (-s2),     0.f,
         inv_scale.x() * (c1 * s2 * s3 - c3 * s1), inv_scale.y() * (c1 * c3 * s2 + s1 * s3), inv_scale.z() * (c1 * c2), 0.f,
         0.f,                                      0.f,                                      0.f,                       1.f;
  return ret;
}

} // namespace hnll::utils