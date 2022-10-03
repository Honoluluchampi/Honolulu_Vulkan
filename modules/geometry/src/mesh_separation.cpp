// hnll
#include <geometry/mesh_separation.hpp>
#include <geometry/half_edge.hpp>

namespace hnll::geometry {

const vec3 RED    = {1.f, 0.f, 0.f};
const vec3 GREEN  = {0.f, 1.f, 0.f};
const vec3 BLUE   = {0.f, 0.f, 1.f};
const vec3 YELLOW = {1.f, 0.5f, 0.f};
const std::vector<vec3> mesh_colors { RED, GREEN, BLUE, YELLOW };

s_ptr<mesh_separation_helper> mesh_separation_helper::create(const s_ptr<mesh_model> &model)
{
  auto helper_sp = std::make_shared<mesh_separation_helper>(model);
  return helper_sp;
}

mesh_separation_helper::mesh_separation_helper(const s_ptr<mesh_model> &model)
  : model_(model), remaining_vertex_map_(model->get_vertex_map()), remaining_face_map_(model->get_face_map())
{

}


s_ptr<meshlet> separate_greedy(const s_ptr<mesh_separation_helper>& helper, const s_ptr<vertex>& current_vertex)
{

}

void colorize_meshlet(const s_ptr<meshlet>& ml)
{
  std::vector<bool> color_flag(mesh_colors.size(), true);
  // search adjoining faces of all faces
  for (const auto& face_kv : ml->get_face_map()) {
    auto first_he = face_kv.second->half_edge_;
    auto current_he = first_he;
    // search all faces adjoining to this face
    while(current_he != first_he) {
      // check all colors
      auto opposite_face = current_he->get_pair()->get_face();
      for (int i = 0; i < mesh_colors.size(); i++) {
        //
        if (mesh_colors[i] == opposite_face->color_)
          color_flag[i] = false;
      }
      current_he = current_he->get_next();
    }
  }

  // extract valid color
  vec3 color;
  for (int i = 0; i < color_flag.size(); i++) {
    if (color_flag[i]) { color = mesh_colors[i]; break; }
  }

  // assign color to vertices and faces
  for (const auto& vert_kv : ml->get_vertex_map())
    vert_kv.second->color_ = color;
  for (const auto& face_kv : ml->get_face_map())
    face_kv.second->color_ = color;
}

std::vector<s_ptr<mesh_model>> mesh_separation_helper::separate(const s_ptr<mesh_model>& model)
{
  auto helper = mesh_separation_helper::create(model);
  // get first vertex
  s_ptr<vertex> current_vertex = nullptr;
  for (const auto& kv : helper->get_remaining_vertex_map()) {
    current_vertex = kv.second;
    break;
  }
  if (current_vertex == nullptr) assert("the model has no vertex!");

  std::vector<s_ptr<meshlet>> mesh_lets;

  // start separation
  while(!helper->face_is_empty()) {
    auto new_mesh_let = separate_greedy(helper, current_vertex);
    mesh_lets.emplace_back(std::move(new_mesh_let));
  }

  // colorization
  for (const auto& ml : mesh_lets)
    colorize_meshlet(ml);

  return mesh_lets;
}
} // namespace hnll::geometry