// hnll
#include <geometry/mesh_separation.hpp>
#include <geometry/half_edge.hpp>

namespace hnll::geometry::mesh_separation {

std::vector<s_ptr<mesh_model>> separate(const s_ptr<mesh_model>& model)
{
  // get first vertex
  auto first_vertex = model->get_vertex();
  std::vector<s_ptr<mesh_model>> res;

  // greedy

}

} // namespace hnll::geometry::mesh_separation