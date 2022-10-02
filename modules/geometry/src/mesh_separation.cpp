// hnll
#include <geometry/mesh_separation.hpp>
#include <geometry/half_edge.hpp>

namespace hnll::geometry {

std::vector<s_ptr<mesh_model>> separate_mesh(const s_ptr<mesh_model>& model)
{
  // get first vertex
  auto first_vertex = model->get_vertex();

}

} // namespace hnll::geometry