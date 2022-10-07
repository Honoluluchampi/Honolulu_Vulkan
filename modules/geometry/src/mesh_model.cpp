// hnll
#include <geometry/mesh_model.hpp>
#include <geometry/half_edge.hpp>
#include <graphics/mesh_model.hpp>
#include <utils/utils.hpp>

// std
#include <filesystem>

// lib
#include <tiny_obj_loader.h>

namespace hnll::geometry {

template <typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
{
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hash_combine(seed, rest), ...);
}

bool operator==(const vertex& rhs, const vertex& lhs)
{
  return (rhs.position_ == lhs.position_)
      && (rhs.color_    == lhs.color_)
      && (rhs.normal_   == lhs.normal_)
      && (rhs.uv_       == lhs.uv_);
}

} // namespace hnll::geometry

namespace std {

template <typename Scalar, int Rows, int Cols>
struct hash<Eigen::Matrix<Scalar, Rows, Cols>> {
  // https://wjngkoh.wordpress.com/2015/03/04/c-hash-function-for-eigen-matrix-and-vector/
  size_t operator()(const Eigen::Matrix<Scalar, Rows, Cols>& matrix) const
  {
    size_t seed = 0;
    for (size_t i = 0; i < matrix.size(); ++i) {
      Scalar elem = *(matrix.data() + i);
      seed ^=
          std::hash<Scalar>()(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

template<>
struct hash<hnll::geometry::vertex>
{
  size_t operator() (hnll::geometry::vertex const& vertex) const
  {
    // stores final hash value
    size_t seed = 0;
    hnll::geometry::hash_combine(seed, vertex.position_); //, vertex.color_, vertex.normal_, vertex.uv_);
    return seed;
  }
};
} // namespace std

namespace hnll::geometry {

s_ptr<vertex> create_vertex_from_pseudo(vertex&& pseudo)
{
  auto vertex_sp = vertex::create(pseudo.position_);
  vertex_sp->color_    = std::move(pseudo.color_);
  vertex_sp->normal_   = std::move(pseudo.normal_);
  vertex_sp->uv_       = std::move(pseudo.uv_);
  return vertex_sp;
}

s_ptr<mesh_model> mesh_model::create_from_obj_file(const std::string& filename)
{
  // get full path
  std::string filepath = "";
  for (const auto& directory : utils::loading_directories) {
    if (std::filesystem::exists(directory + "/" + filename))
      filepath = directory + "/" + filename;
  }
  if (filepath == "")
    std::runtime_error(filename + " doesn't exist!");

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
    throw std::runtime_error(warn + err);

  auto mesh_model = mesh_model::create();

  std::unordered_map<vec3, vertex_id> unique_vertices_position{};
  std::vector<vertex_id> indices;

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      // does not have id_
      vertex vertex{};
      // copy the vertex
      if (index.vertex_index >= 0) {
        vertex.position_ = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2]
        };
        // color support
        vertex.color_ = {
          attrib.colors[3 * index.vertex_index + 0],
          attrib.colors[3 * index.vertex_index + 1],
          attrib.colors[3 * index.vertex_index + 2]
        };
      }
      // copy the normal
      if (index.vertex_index >= 0) {
        vertex.normal_ = {
          attrib.normals[3 * index.normal_index + 0],
          attrib.normals[3 * index.normal_index + 1],
          attrib.normals[3 * index.normal_index + 2]
        };
      }
      // copy the texture coordinate
      if (index.vertex_index >= 0) {
        vertex.uv_ = {
          attrib.vertices[2 * index.texcoord_index + 0],
          attrib.vertices[2 * index.texcoord_index + 1]
        };
      }
      // if vertex is a new vertex
      if (unique_vertices_position.find(vertex.position_) == unique_vertices_position.end()) {
        auto new_vertex = create_vertex_from_pseudo(std::move(vertex));
        auto new_id = mesh_model->add_vertex(new_vertex);
        unique_vertices_position[new_vertex->position_] = new_id;
        indices.push_back(new_id);
      }
      else
        indices.push_back(unique_vertices_position[vertex.position_]);
    }
  }

  if (indices.size() % 3 != 0)
    throw std::runtime_error("vertex count is not multiple of 3");
  // recreate all faces
  for (int i = 0; i < indices.size(); i += 3) {
    auto v0 = mesh_model->get_vertex(indices[i]);
    auto v1 = mesh_model->get_vertex(indices[i + 1]);
    auto v2 = mesh_model->get_vertex(indices[i + 2]);
    auto normal = (v0->normal_ + v1->normal_ + v2->normal_) / 3.f;
    auto cross = (v1->position_ - v0->position_).cross(v2->position_ - v0->position_);
    if (cross.dot(normal) >= 0)
      mesh_model->add_face(v0, v1, v2);
    else
      mesh_model->add_face(v0, v2, v1);
  }

  return mesh_model;
}

void mesh_model::align_vertex_id()
{
  vertex_map new_map;
  vertex_id new_id = 0;
  for (const auto& kv : vertex_map_) {
    kv.second->id_ = new_id;
    new_map[new_id++] = kv.second;
  }
  vertex_map_ = new_map;
}

void mesh_model::colorize_whole_mesh(const vec3& color)
{ for (const auto& kv : vertex_map_) kv.second->color_ = color; }

half_edge_key calc_half_edge_key(const s_ptr<vertex>& v0, const s_ptr<vertex>& v1)
{
  half_edge_key id0 = v0->id_, id1 = v1->id_;
  return (id0 | (id1 << 32));
}

half_edge_key calc_pair_half_edge_key(const s_ptr<vertex>& v0, const s_ptr<vertex>& v1)
{
  half_edge_key id0 = v0->id_, id1 = v1->id_;
  return (id1 | (id0 << 32));
}

s_ptr<half_edge> mesh_model::get_half_edge(const s_ptr<vertex> &v0, const s_ptr<vertex> &v1)
{
  auto hash_key = calc_half_edge_key(v0, v1);
  if (half_edge_map_.find(hash_key) != half_edge_map_.end())
    return half_edge_map_[hash_key];
  return nullptr;
}

bool mesh_model::associate_half_edge_pair(const s_ptr<half_edge> &he)
{
  auto hash_key = calc_half_edge_key(he->get_vertex(), he->get_next()->get_vertex());
  auto pair_hash_key = calc_pair_half_edge_key(he->get_vertex(), he->get_next()->get_vertex());

  half_edge_map_[hash_key] = he;
  // check if those hash_key already have a value
  if (half_edge_map_.find(pair_hash_key) != half_edge_map_.end()) {
    // if the pair has added to the map, associate with it
    he->set_pair(half_edge_map_[pair_hash_key]);
    half_edge_map_[pair_hash_key]->set_pair(he);
    return true;
    }
//  } else {
    // if the pair has not added to the map
//    half_edge_map_[hash_key] = he;
    return false;
//  }
}

vertex_id mesh_model::add_vertex(const s_ptr<vertex> &v)
{
  // if the vertex has not been involved
  if (vertex_map_.find(v->id_) == vertex_map_.end())
    vertex_map_[v->id_] = v;
  return v->id_;
}

face_id mesh_model::add_face(s_ptr<vertex> &v0, s_ptr<vertex> &v1, s_ptr<vertex> &v2, auto_vertex_normal_calculation avnc)
{
  // register to the vertex hash table
  add_vertex(v0);
  add_vertex(v1);
  add_vertex(v2);
  // create new half_edge
  std::array<s_ptr<half_edge>, 3> hes;
  hes[0] = half_edge::create(v0);
  hes[1] = half_edge::create(v1);
  hes[2] = half_edge::create(v2);
  // half_edge circle
  hes[0]->set_next(hes[1]); hes[0]->set_prev(hes[2]);
  hes[1]->set_next(hes[2]); hes[0]->set_prev(hes[0]);
  hes[2]->set_next(hes[0]); hes[0]->set_prev(hes[1]);
  // register to the hash_table
  for (int i = 0; i < 3; i++) {
    associate_half_edge_pair(hes[i]);
  }
  // new face
  auto fc = face::create(hes[0]);
  // calc face normal
  fc->normal_ = ((v1->position_ - v0->position_).cross(v2->position_ - v0->position_)).normalized();
  // register to each owner
  face_map_[fc->id_] = fc;
  hes[0]->set_face(fc);
  hes[1]->set_face(fc);
  hes[2]->set_face(fc);
  // vertex normal
  if (avnc == auto_vertex_normal_calculation::ON) {
    v0->update_normal(fc->normal_);
    v1->update_normal(fc->normal_);
    v2->update_normal(fc->normal_);
  }
  else {
    v0->face_count_++;
    v1->face_count_++;
    v2->face_count_++;
  }
  return fc->id_;
}

} // namespace hnll::geometry