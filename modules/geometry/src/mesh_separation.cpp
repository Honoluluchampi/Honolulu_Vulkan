// hnll
#include <geometry/mesh_separation.hpp>
#include <geometry/half_edge.hpp>

namespace hnll::geometry {

mesh_separation_helper::mesh_separation_helper(const s_ptr<mesh_model> &model)
  : remaining_vertex_map_(model->get_vertex_map()), remaining_face_map_(model->get_face_map())
{

}

namespace mesh_separation {

using mesh_let = mesh_model;

std::vector<s_ptr<mesh_model>> separate(const s_ptr<mesh_model>& model)
{
  mesh_separation_helper helper {model};
  // get first vertex
  s_ptr<vertex> current_vertex = nullptr;
  for (const auto& kv : helper.get_remaining_vertex_map()) {
    current_vertex = kv.second;
    break;
  }
  if (current_vertex == nullptr) assert("the model has no vertex!");

  std::vector<s_ptr<mesh_model>> res;
  // greedy

}

} // namespace mesh_separation
} // namespace hnll::geometry