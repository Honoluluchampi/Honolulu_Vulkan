// hnll
#include <geometry/mesh_separation.hpp>
#include <geometry/half_edge.hpp>
#include <geometry/bounding_volume.hpp>

namespace hnll::geometry {

const vec3 RED    = {1.f, 0.f, 0.f};
const vec3 GREEN  = {0.f, 1.f, 0.f};
const vec3 BLUE   = {0.f, 0.f, 1.f};
const vec3 YELLOW = {1.f, 0.5f, 0.f};
const std::vector<vec3> mesh_colors { RED, GREEN, BLUE, YELLOW };

s_ptr<face> mesh_separation_helper::get_random_face()
{
  for (const auto& kv : remaining_face_map_) {
    return kv.second;
  }
  return nullptr;
}

s_ptr<mesh_separation_helper> mesh_separation_helper::create(const s_ptr<mesh_model> &model)
{
  auto helper_sp = std::make_shared<mesh_separation_helper>(model);
  return helper_sp;
}

mesh_separation_helper::mesh_separation_helper(const s_ptr<mesh_model> &model)
  : model_(model), remaining_vertex_map_(model->get_vertex_map()), remaining_face_map_(model->get_face_map())
{
}

u_ptr<geometry::bounding_volume> create_aabb_from_single_face(const s_ptr<face>& fc)
{
  std::vector<vec3> vertices;
  auto first_he = fc->half_edge_;
  auto current_he = first_he;
  do {
    vertices.push_back(current_he->get_vertex()->position_);
    current_he = current_he->get_next();
  } while (current_he != first_he);
  return geometry::bounding_volume::create_aabb(vertices);
}

double compute_loss_function(const bounding_volume& current_aabb, const s_ptr<face>& new_face)
{

}

s_ptr<face> choose_the_best_face(const face_map& adjoining_face_map, const bounding_volume& aabb)
{
  // TODO : get loss function from caller
  s_ptr<face> res;
  // minimize this loss_value
  double loss_value = 1e9 + 7;
  for (const auto& fc_kv : adjoining_face_map) {
    if (compute_loss_function(aabb, fc_kv.second) < loss_value)
      res = fc_kv.second;
  }
  return res;
}



std::vector<s_ptr<meshlet>> separate_greedy(const s_ptr<mesh_separation_helper>& helper)
{
  std::vector<s_ptr<meshlet>> mesh_lets;
  s_ptr<face> current_face = helper->get_random_face();

  while (!helper->face_is_empty()) {
    // compute each meshlet
    // init objects
    s_ptr<meshlet> ml;
    auto aabb = create_aabb_from_single_face(current_face);
    face_map adjoining_face_map {{current_face->id_ ,current_face}};
    while (ml->get_vertex_count() < mesh_separation::VERTEX_COUNT_PER_MESHLET
        && ml->get_face_count() < mesh_separation::PRIMITIVE_COUNT_PER_MESHLET ) {

      current_face = choose_the_best_face(adjoining_face_map, *aabb);
      // add current_face to ml
      // update aabb (current_face)
      // update adjoining_face_map(current_face)
    }
    mesh_lets.emplace_back(std::move(ml));
    // current_face = random face of adjoining_face_map
  }

  return mesh_lets;
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

  std::vector<s_ptr<meshlet>> mesh_lets = separate_greedy(helper);

  // colorization
  for (const auto& ml : mesh_lets)
    colorize_meshlet(ml);

  return mesh_lets;
}
} // namespace hnll::geometry