#pragma once

// hnll
#include <geometry/half_edge.hpp>

// std
#include <memory>
#include <unordered_map>
#include <string>

//lib
#include <eigen3/Eigen/Dense>

// hash functions
namespace hnll::geometry {
template<typename T, typename... Rest>
void hash_combine(std::size_t &seed, const T &v, const Rest &... rest) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hash_combine(seed, rest), ...);
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

// hash for half edge map
template<>
struct hash<std::pair<hnll::geometry::vertex, hnll::geometry::vertex>>
{
  size_t operator() (const std::pair<hnll::geometry::vertex, hnll::geometry::vertex>& vertex_pair) const
  {
    size_t seed = 0;
    // only positions of vertex matter for half edge
    hnll::geometry::hash_combine(seed, vertex_pair.first.position_, vertex_pair.second.position_);
    return seed;
  }
};

} // namespace std

// forward declaration
namespace hnll::graphics { class mesh_model; struct mesh_builder; }
namespace hnll::geometry {

// forward declaration
class  bounding_volume;
enum class bv_type;

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

// for adding_face creation
enum class auto_vertex_normal_calculation { ON, OFF };

using vec3          = Eigen::Vector3d;
using vertex_id     = uint32_t;
using vertex_map    = std::unordered_map<vertex_id, s_ptr<vertex>>;
using face_id       = uint32_t;
using face_map      = std::unordered_map<face_id, s_ptr<face>>;
using half_edge_key = std::pair<vertex, vertex>;
using half_edge_map = std::unordered_map<half_edge_key, s_ptr<half_edge>>;

class mesh_model
{
  public:
    static s_ptr<mesh_model> create();
    static s_ptr<mesh_model> create_from_obj_file(const std::string& filename);
    static s_ptr<mesh_model> create_from_mesh_builder(graphics::mesh_builder&& builder);

    mesh_model();
    void align_vertex_id();

    // vertices are assumed to be in a counter-clockwise order
    vertex_id add_vertex(const s_ptr<vertex>& v);
    face_id   add_face(s_ptr<vertex>& v0, s_ptr<vertex>& v1, s_ptr<vertex>& v2,
      geometry::auto_vertex_normal_calculation avnc = geometry::auto_vertex_normal_calculation::OFF);

    // getter
    vertex_map       get_vertex_map() const         { return vertex_map_; }
    face_map         get_face_map() const           { return face_map_; }
    half_edge_map    get_half_edge_map() const      { return half_edge_map_; }
    size_t           get_face_count() const         { return face_map_.size(); }
    size_t           get_vertex_count() const       { return vertex_map_.size(); }
    size_t           get_half_edge_count() const    { return half_edge_map_.size(); }
    s_ptr<face>      get_face(const face_id id)     { return face_map_[id]; }
    s_ptr<vertex>    get_vertex(const vertex_id id) { return vertex_map_[id]; }
    const bounding_volume& get_bounding_volume() const;
    u_ptr<bounding_volume> get_bounding_volume_copy() const;
    u_ptr<bounding_volume> get_ownership_of_bounding_volume();

    // setter
    void set_bounding_volume(u_ptr<bounding_volume>&& bv);
    void colorize_whole_mesh(const vec3& color);
    void set_bv_type(bv_type type);
  private:
    // returns false if the pair have not been registered to the map
    bool associate_half_edge_pair(const s_ptr<half_edge>& he);

    half_edge_map half_edge_map_;
    face_map      face_map_;
    vertex_map    vertex_map_;
    u_ptr<bounding_volume> bounding_volume_;
};



} // namespace hnll::geometry