// hnll
#include <geometry/mesh_separation.hpp>
#include <geometry/primitives.hpp>
#include <geometry/mesh_model.hpp>
#include <geometry/bounding_volume.hpp>
#include <geometry/intersection.hpp>
#include <graphics/meshlet_model.hpp>
#include <graphics/utils.hpp>
#include <utils/utils.hpp>

// std
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

namespace hnll::geometry {

std::vector<ray> create_sampling_rays(const face &_face, uint32_t _sampling_count)
{
  std::vector<ray> sampling_rays;

  // returns no ray
  if (_sampling_count <= 0) return sampling_rays;

  auto& v0 = _face.half_edge_->get_vertex()->position_;
  auto& v1 = _face.half_edge_->get_next()->get_vertex()->position_;
  auto& v2 = _face.half_edge_->get_next()->get_next()->get_vertex()->position_;

  auto  origin = (v0 + v1 + v2) / 3.f;

  // default ray
  sampling_rays.emplace_back(ray{origin, _face.normal_});

  for (int i = 0; i < _sampling_count - 1; i++) {

  }

  return sampling_rays;
}

// returns -1 if the ray doesn't intersect with any triangle
double mesh_separation_helper::compute_shape_diameter(const ray& _ray)
{
  for (const auto& f_kv : face_map_) {
    auto& he = f_kv.second->half_edge_;
    std::vector<vec3d> vertices = {
      he->get_vertex()->position_,
      he->get_next()->get_vertex()->position_,
      he->get_next()->get_next()->get_vertex()->position_,
    };

    intersection::test_ray_triangle(_ray, vertices);
  }
  return -1;
}

void mesh_separation_helper::compute_whole_shape_diameters()
{
  for (auto& f_kv : face_map_) {
    auto& f = *f_kv.second;

    // compute shape diameter
    auto sampling_rays = create_sampling_rays(f, 1);

    double sdf_mean = 0.f;

    // compute values for each sampling rays
    for (const auto& r : sampling_rays) {
      if (auto tmp = compute_shape_diameter(r); tmp != -1)
        sdf_mean += compute_shape_diameter(r);
    }

    sdf_mean /= sampling_rays.size();

    f.shape_diameter_ = sdf_mean;
  }
}

std::vector<mesh_model> mesh_separation_helper::separate_using_sdf()
{

}

s_ptr<face> mesh_separation_helper::get_random_remaining_face()
{
  for (const auto& id : remaining_face_id_set_) {
    return face_map_[id];
  }
  return nullptr;
}

s_ptr<mesh_separation_helper> mesh_separation_helper::create(
  const s_ptr<mesh_model> &model,
  const std::string& _model_name,
  mesh_separation::criterion _crtr)
{
  auto helper_sp = std::make_shared<mesh_separation_helper>(model);
  helper_sp->set_model_name(_model_name);
  helper_sp->set_criterion(_crtr);
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
  std::cout << "odd h-edge count : " << absent_pair_count << std::endl;
  std::cout << "vertex count : " << vertex_map_.size() << std::endl;
  std::cout << "face count : " << face_map_.size() << std::endl;
  std::cout << "h-edge count : " << model_->get_half_edge_map().size() << std::endl;
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

u_ptr<geometry::bounding_volume> create_b_sphere_from_single_face(const s_ptr<face>& fc)
{
  std::vector<vec3> vertices;
  auto first_he = fc->half_edge_;
  auto current_he = first_he;
  do {
    vertices.push_back(current_he->get_vertex()->position_);
    current_he = current_he->get_next();
  } while (current_he != first_he);
  return geometry::bounding_volume::create_bounding_sphere(bv_ctor_type::RITTER, vertices);
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
  auto x_diff = max_x - min_x;
  auto y_diff = max_y - min_y;
  auto z_diff = max_z - min_z;

  auto volume = x_diff * y_diff * z_diff;

  auto mean = (x_diff + y_diff + z_diff) / 3.f;
  auto variance = (std::pow(x_diff - mean, 2) + std::pow(y_diff - mean, 2) + std::pow(z_diff - mean, 2)) / 3.f;
  return volume + variance;
}

double calc_dist(const vec3& a, const vec3& b)
{
  auto ba = b - a;
  return ba.x() * ba.x() + ba.y() * ba.y() + ba.z() * ba.z();
}

double compute_loss_function_for_sphere(const bounding_volume& current_sphere, const s_ptr<face>& new_face)
{
  auto he = new_face->half_edge_;
  auto radius2 = std::pow(current_sphere.get_sphere_radius(), 2);
  radius2 = std::max(radius2, calc_dist(current_sphere.get_local_center_point(), he->get_vertex()->position_));
  radius2 = std::max(radius2, calc_dist(current_sphere.get_local_center_point(), he->get_next()->get_vertex()->position_));
  radius2 = std::max(radius2, calc_dist(current_sphere.get_local_center_point(), he->get_next()->get_next()->get_vertex()->position_));
  return radius2;
}

s_ptr<face> choose_the_best_face(const face_map& adjoining_face_map, const bounding_volume& aabb)
{
  // TODO : get loss function from caller
  s_ptr<face> res;
  // minimize this loss_value
  double loss_value = 1e9 + 7;
  for (const auto& fc_kv : adjoining_face_map) {
    if (auto new_loss = compute_loss_function(aabb, fc_kv.second); new_loss < loss_value) {
      loss_value = new_loss;
      res = fc_kv.second;
    }
  }
  return res;
}

s_ptr<face> choose_the_best_face_for_sphere(const face_map& adjoining_face_map, const bounding_volume& sphere)
{
  // TODO : get loss function from caller
  s_ptr<face> res;
  // minimize this loss_value
  double loss_value = 1e9 + 7;
  for (const auto& fc_kv : adjoining_face_map) {
    if (auto new_loss = compute_loss_function_for_sphere(sphere, fc_kv.second); new_loss < loss_value) {
      loss_value = new_loss;
      res = fc_kv.second;
    }
  }
  return res;
}

void add_face_to_meshlet(const s_ptr<face>& fc, s_ptr<mesh_model>& ml)
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

void update_sphere(bounding_volume& current_sphere, const s_ptr<face>& new_face)
{
  // calc farthest point
  int far_id = -1;
  auto he = new_face->half_edge_;
  auto far_dist2 = std::pow(current_sphere.get_sphere_radius(), 2);
  if (auto dist = calc_dist(current_sphere.get_local_center_point(), he->get_vertex()->position_); dist > far_dist2 ){
    far_dist2 = dist;
    far_id = 0;
  }
  if (auto dist = calc_dist(current_sphere.get_local_center_point(), he->get_next()->get_vertex()->position_); dist > far_dist2 ){
    far_dist2 = dist;
    far_id = 1;
  }
  if (auto dist = calc_dist(current_sphere.get_local_center_point(), he->get_next()->get_next()->get_vertex()->position_); dist > far_dist2 ){
    far_dist2 = dist;
    far_id = 2;
  }

  if (far_id == -1) return;

  vec3 new_position;
  for (int i = 0; i < far_id; i++) {
    he = he->get_next();
  }
  new_position = he->get_vertex()->position_;

  // compute new center and radius
  auto center = current_sphere.get_local_center_point();
  auto radius = current_sphere.get_sphere_radius();
  auto new_dir = (new_position - center).normalized();
  auto opposite = center - radius * new_dir;
  auto new_center = 0.5f * (new_position + opposite);
  current_sphere.set_center_point(new_center);

  auto new_radius = std::sqrt(calc_dist(new_center, new_position));
  current_sphere.set_sphere_radius(new_radius);
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

graphics::meshlet translate_meshlet(const s_ptr<mesh_model>& old_mesh)
{
  graphics::meshlet ret{};

  std::unordered_map<uint32_t, uint32_t> id_map;
  uint32_t current_id = 0;

  // fill vertex info
  for (const auto& v_kv : old_mesh->get_vertex_map()) {
    auto& v = v_kv.second;
    // register to id_map
    id_map[v->id_] = current_id;
    // add to meshlet
    ret.vertex_indices[current_id++] = v->id_;
  }
  ret.vertex_count = current_id;

  current_id = 0;
  // fill primitive index info
  for (const auto& f_kv : old_mesh->get_face_map()) {
    auto& f = f_kv.second;
    auto& he = f->half_edge_;
    for (int i = 0; i < 3; i++) {
      ret.primitive_indices[current_id++] = id_map[he->get_vertex()->id_];
      he = he->get_next();
    }
  }
  ret.index_count = current_id;

  // fill bounding volume info
  ret.center = old_mesh->get_bounding_volume().get_local_center_point().cast<float>();
  ret.radius = old_mesh->get_bounding_volume().get_sphere_radius();

  return ret;
}

std::vector<graphics::meshlet> separate_greedy(const s_ptr<mesh_separation_helper>& helper)
{
  std::vector<graphics::meshlet> meshlets;
  auto crtr = helper->get_criterion();
  s_ptr<face> current_face = helper->get_random_remaining_face();

  while (!helper->all_face_is_registered()) {
    // compute each meshlet
    // init objects
    s_ptr<mesh_model> ml = mesh_model::create();

    // change functions depending on the criterion
    u_ptr<bounding_volume> bv;
    if (crtr == mesh_separation::criterion::MINIMIZE_AABB) bv = create_aabb_from_single_face(current_face);
    if (crtr == mesh_separation::criterion::MINIMIZE_BOUNDING_SPHERE) bv = create_b_sphere_from_single_face(current_face);

    face_map adjoining_face_map {{current_face->id_ ,current_face}};

    // meshlet api limitation
    while (ml->get_vertex_count() < mesh_separation::VERTEX_COUNT_PER_MESHLET
        && ml->get_face_count() < mesh_separation::PRIMITIVE_COUNT_PER_MESHLET
        && adjoining_face_map.size() != 0) {

      // algorithm dependent part
      if (crtr == mesh_separation::criterion::MINIMIZE_AABB)
        current_face = choose_the_best_face(adjoining_face_map, *bv);
      if (crtr == mesh_separation::criterion::MINIMIZE_BOUNDING_SPHERE)
        current_face = choose_the_best_face_for_sphere(adjoining_face_map, *bv);
      // update each object
      add_face_to_meshlet(current_face, ml);
      if (crtr == mesh_separation::criterion::MINIMIZE_AABB)
        update_aabb(*bv, current_face);
      if (crtr == mesh_separation::criterion::MINIMIZE_BOUNDING_SPHERE)
        update_sphere(*bv, current_face);
      helper->update_adjoining_face_map(adjoining_face_map, current_face);
      helper->remove_face(current_face->id_);
    }
    ml->set_bounding_volume(std::move(bv));
    meshlets.emplace_back(translate_meshlet(ml));
    current_face = choose_random_face_from_map(adjoining_face_map);
    if (current_face == nullptr)
      current_face = helper->get_random_remaining_face();
  }

  return meshlets;
}

std::vector<graphics::meshlet> mesh_separation::separate(
  const s_ptr<mesh_model>& _model,
  const std::string& _model_name,
  criterion _crtr)
{
  std::vector<graphics::meshlet> meshlets;

  auto helper = mesh_separation_helper::create(_model, _model_name, _crtr);

  meshlets = separate_greedy(helper);

  write_meshlet_cache(meshlets, helper->get_model_name(), helper->get_criterion());

  return meshlets;
}

std::vector<graphics::meshlet> mesh_separation::separate_without_cache(
  const s_ptr<mesh_model>& _model,
  criterion _crtr)
{
  std::vector<graphics::meshlet> meshlets;

  auto helper = mesh_separation_helper::create(_model, "", _crtr);

  meshlets = separate_greedy(helper);

  return meshlets;
}

void mesh_separation::write_meshlet_cache(
  const std::vector<graphics::meshlet> &_meshlets,
  const std::string& _filename,
  criterion _crtr)
{
  auto directory = utils::create_sub_cache_directory("meshlets");

  std::ofstream writing_file;
  std::string filepath = directory + "/" + _filename + ".ml";
  writing_file.open(filepath, std::ios::out);

  // write contents
  writing_file << filepath << std::endl;
  writing_file << "greedy" << std::endl;
  switch (_crtr) {
    case criterion::MINIMIZE_BOUNDING_SPHERE :
      writing_file << "MINIMIZE_BOUNDING_SPHERE" << std::endl;
      break;
    case criterion::MINIMIZE_AABB :
      writing_file << "MINIMIZE_AABB" << std::endl;
      break;
    default :
      ;
  }

  auto meshlet_count = _meshlets.size();
  writing_file << meshlet_count << std::endl;
  for (int i = 0; i < meshlet_count; i++) {
    auto current_ml = _meshlets[i];
    // vertex info
    writing_file << current_ml.vertex_count << std::endl;
    for (const auto& v_id : current_ml.vertex_indices) {
      writing_file << v_id << ",";
    }
    writing_file << std::endl;
    // face info
    writing_file << current_ml.index_count << std::endl;
    for (const auto& i_id : current_ml.primitive_indices) {
      writing_file << i_id << ",";
    }
    writing_file << std::endl;
    // bonding volume info
    writing_file << current_ml.center.x() << ',' <<
      current_ml.center.y() << ',' <<
      current_ml.center.z() << std::endl;
    writing_file << current_ml.radius << std::endl;
  }
  writing_file.close();
}

bool mesh_separation::load_meshlet_cache(const std::string &_filename, std::vector<graphics::meshlet>& meshlets)
{
  std::string cache_dir = std::string(getenv("HNLL_ENGN")) + "/cache/meshlets/";
  std::string file_path = cache_dir + _filename + ".ml";

  // cache does not exist
  if (!std::filesystem::exists(file_path)) {
    return false;
  }

  std::ifstream reading_file(file_path);
  std::string buffer;

  if (reading_file.fail()) {
    throw std::runtime_error("failed to open file" + file_path);
  }

  // ignore first three lines
  for (int i = 0; i < 4; i++) {
    getline(reading_file, buffer);
  }

  // 4th line indicates the meshlet count
  uint32_t meshlet_count = std::stoi(buffer);
  meshlets.resize(meshlet_count);

  // read info
  for (int i = 0; i < meshlet_count; i++) {
    // vertex count
    getline(reading_file, buffer);
    meshlets[i].vertex_count = std::stoi(buffer);
    // vertex indices array
    for (int j = 0; j < graphics::MAX_VERTEX_PER_MESHLET; j++) {
      getline(reading_file, buffer, ',');
      meshlets[i].vertex_indices[j] = std::stoi(buffer);
    }
    getline(reading_file, buffer);
    // index count
    getline(reading_file, buffer);
    meshlets[i].index_count = std::stoi(buffer);
    // primitive indices array
    for (int j = 0; j < graphics::MAX_INDEX_PER_MESHLET; j++) {
      getline(reading_file, buffer, ',');
      meshlets[i].primitive_indices[j] = std::stoi(buffer);
    }
    getline(reading_file, buffer);
    // bounding volume
    // center
    getline(reading_file, buffer, ',');
    meshlets[i].center.x() = std::stof(buffer);
    getline(reading_file, buffer, ',');
    meshlets[i].center.y() = std::stof(buffer);
    getline(reading_file, buffer);
    meshlets[i].center.z() = std::stof(buffer);
    // radius
    getline(reading_file, buffer);
    meshlets[i].radius = std::stof(buffer);
  }

  return true;
}

} // namespace hnll::geometry