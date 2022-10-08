// hnll
#include <geometry/perspective_frustum.hpp>
#include <utils/utils.hpp>

namespace hnll::geometry {

void perspective_frustum::update_plane_normals(const utils::transform &tf)
{
  const auto translation = tf.translation;
  const auto rotate_mat = tf.rotate_mat3();

}

} // namespace hnll::geometry