// hnll
#include <geometry/mesh_separation.hpp>
#include <geometry/half_edge.hpp>
#include <geometry/mesh_model.hpp>
#include <geometry/bounding_volume.hpp>

#include <iostream>

namespace hnll::geometry {

const vec3 RED    = {1.f, 0.f, 0.f};
const vec3 GREEN  = {0.f, 1.f, 0.f};
const vec3 BLUE   = {0.f, 0.f, 1.f};
const vec3 YELLOW = {1.f, 0.5f, 0.f};
const vec3 LIGHT_BLUE = {0.3f, 0.7f, 0.7f};
const std::vector<vec3> mesh_colors { GREEN, RED, BLUE, YELLOW, LIGHT_BLUE };

s_ptr<face> mesh_separation_helper::get_random_remaining_face()
{
  for (const auto& id : remaining_face_id_set_) {
    return face_map_[id];
  }
  return nullptr;
}

s_ptr<mesh_separation_helper> mesh_separation_helper::create(const s_ptr<mesh_model> &model)
{
  auto helper_sp = std::make_shared<mesh_separation_helper>(model);
  return helper_sp;
}

mesh_separation_helper::mesh_separation_helper(const s_ptr<mesh_model> &model)
  : model_(model), vertex_map_(model->get_vertex_map()), face_map_(model->get_face_map())
{
  for (const auto& fc_kv : face_map_) {
    remaining_face_id_set_.insert(fc_kv.first);
  }

  unsigned int absent_pair_count = 0;
  for (const auto& he_kv : model_->get_half_edge_map()) {
    if (he_kv.second->get_pair() == nullptr) {
      absent_pair_count++;
    }
  }
  std::cout << absent_pair_count << std::endl;
  std::cout << vertex_map_.size() << std::endl;
  std::cout << face_map_.size() << std::endl;
  std::cout << model_->get_half_edge_map().size() << std::endl;
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
  auto face_aabb = create_aabb_from_single_face(new_face);
  double max_x = std::max(current_aabb.get_max_x(), face_aabb->get_max_x());
  double min_x = std::min(current_aabb.get_min_x(), face_aabb->get_min_x());
  double max_y = std::max(current_aabb.get_max_y(), face_aabb->get_max_y());
  double min_y = std::min(current_aabb.get_min_y(), face_aabb->get_min_y());
  double max_z = std::max(current_aabb.get_max_z(), face_aabb->get_max_z());
  double min_z = std::min(current_aabb.get_min_z(), face_aabb->get_min_z());
  return (max_x - min_x) * (max_y - min_y) * (max_z - min_z);
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

void add_face_to_meshlet(const s_ptr<face>& fc, s_ptr<meshlet>& ml)
{
  auto he = fc->half_edge_;
  auto v0 = he->get_vertex();
  he = he->get_next();
  auto v1 = he->get_vertex();
  he = he->get_next();
  auto v2 = he->get_vertex();
  ml->add_face(v0, v1, v2);
}

void update_aabb(bounding_volume& current_aabb, const s_ptr<face>& new_face)
{
  auto face_aabb = create_aabb_from_single_face(new_face);
  vec3 max_vec3, min_vec3;
  max_vec3.x() = std::max(current_aabb.get_max_x(), face_aabb->get_max_x());
  min_vec3.x() = std::min(current_aabb.get_min_x(), face_aabb->get_min_x());
  max_vec3.y() = std::max(current_aabb.get_max_y(), face_aabb->get_max_y());
  min_vec3.y() = std::min(current_aabb.get_min_y(), face_aabb->get_min_y());
  max_vec3.z() = std::max(current_aabb.get_max_z(), face_aabb->get_max_z());
  min_vec3.z() = std::min(current_aabb.get_min_z(), face_aabb->get_min_z());
  auto center_vec3 = (max_vec3 + min_vec3) / 2.f;
  auto radius_vec3 = max_vec3 - center_vec3;
  current_aabb.set_center_point(center_vec3);
  current_aabb.set_aabb_radius(radius_vec3);
}

void mesh_separation_helper::update_adjoining_face_map(face_map& adjoining_face_map, const s_ptr<face>& fc)
{
  auto first_he = fc->half_edge_;
  auto current_he = first_he;
  do {
    auto he_pair = current_he->get_pair();
    if (he_pair != nullptr) {
      auto current_face = current_he->get_pair()->get_face();
      // if current_face is new to the map
      if (remaining_face_id_set_.find(current_face->id_) != remaining_face_id_set_.end()) {
        adjoining_face_map[current_face->id_] = current_face;
      }
    }
    current_he = current_he->get_next();
  } while (current_he != first_he);
  adjoining_face_map.erase(fc->id_);
}

s_ptr<face> choose_random_face_from_map(const face_map& fc_map)
{
  for (const auto& fc_kv : fc_map) {
    return fc_kv.second;
  }
  return nullptr;
}

s_ptr<vertex> duplicate_vertex(const s_ptr<vertex>& old_vertex)
{
  auto new_vertex = vertex::create(old_vertex->position_);
  new_vertex->position_ = old_vertex->position_;
  new_vertex->normal_ = old_vertex->normal_;
  new_vertex->color_ = old_vertex->color_;
  new_vertex->uv_ = old_vertex->uv_;
  new_vertex->half_edge_ = old_vertex->half_edge_;
  new_vertex->face_count_ = old_vertex->face_count_;
  return new_vertex;
}

s_ptr<meshlet> recreate_meshlet(const s_ptr<meshlet>& old_mesh)
{
  auto new_mesh = mesh_model::create();

  for (const auto& f_kv : old_mesh->get_face_map()) {
    auto he = f_kv.second->half_edge_;
    auto v0 = duplicate_vertex(he->get_vertex());
    auto v1 = duplicate_vertex(he->get_next()->get_vertex());
    auto v2 = duplicate_vertex(he->get_next()->get_next()->get_vertex());
    new_mesh->add_face(v0, v1, v2);
  }
  return new_mesh;
}

std::vector<s_ptr<meshlet>> separate_greedy(const s_ptr<mesh_separation_helper>& helper)
{
  std::vector<s_ptr<meshlet>> meshlets;
  s_ptr<face> current_face = helper->get_random_remaining_face();

  while (!helper->all_face_is_registered()) {
    // compute each meshlet
    // init objects
    s_ptr<meshlet> ml = mesh_model::create();
    auto aabb = create_aabb_from_single_face(current_face);
    face_map adjoining_face_map {{current_face->id_ ,current_face}};
    // meshlet api limitation
    while (ml->get_vertex_count() < mesh_separation::VERTEX_COUNT_PER_MESHLET
        && ml->get_face_count() < mesh_separation::PRIMITIVE_COUNT_PER_MESHLET
        && adjoining_face_map.size() != 0) {

      // algorithm dependent part
      current_face = choose_the_best_face(adjoining_face_map, *aabb);
      // update each object
      add_face_to_meshlet(current_face, ml);
      update_aabb(*aabb, current_face);
      helper->update_adjoining_face_map(adjoining_face_map, current_face);
      helper->remove_face(current_face->id_);
    }
    meshlets.emplace_back(recreate_meshlet(ml));
    current_face = choose_random_face_from_map(adjoining_face_map);
    if (current_face == nullptr)
      current_face = helper->get_random_remaining_face();
  }

  return meshlets;
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

  static int i = 0;
  auto new_color = mesh_colors[i++ % mesh_colors.size()];
  // assign color to vertices and faces
  for (const auto& vert_kv : ml->get_vertex_map())
    vert_kv.second->color_ = new_color;
  for (const auto& face_kv : ml->get_face_map())
    face_kv.second->color_ = new_color;
}

std::vector<s_ptr<mesh_model>> mesh_separation::separate(const s_ptr<mesh_model>& model)
{
  auto helper = mesh_separation_helper::create(model);

  std::vector<s_ptr<meshlet>> mesh_lets = separate_greedy(helper);

  // colorization
  for (const auto& ml : mesh_lets)
    colorize_meshlet(ml);

  std::cout << mesh_lets.size() << std::endl;
  return mesh_lets;
}
} // namespace hnll::geometry