#pragma once

// std
#include <memory>
#include <unordered_map>
#include <string>

//lib
#include <eigen3/Eigen/Dense>

// forward declaration
namespace hnll::graphics { class mesh_model; struct builder; }
namespace hnll::geometry {

// forward declaration
struct vertex;
struct face;
class  half_edge;

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

// for adding_face creation
enum class auto_vertex_normal_calculation { ON, OFF };

using vec3          = Eigen::Vector3d;
using vertex_id     = uint32_t;
using vertex_map    = std::unordered_map<vertex_id, s_ptr<vertex>>;
using face_id       = uint32_t;
using face_map      = std::unordered_map<face_id, s_ptr<face>>;
using half_edge_key = uint64_t; // consists of two vertex_ids
using half_edge_map = std::unordered_map<half_edge_key, s_ptr<half_edge>>;

class mesh_model
{
  public:
    static s_ptr<mesh_model> create() { return std::make_shared<mesh_model>(); }
    static s_ptr<mesh_model> create_from_obj_file(const std::string& filename);

    void align_vertex_id();

    // vertices are assumed to be in a counter-clockwise order
    vertex_id add_vertex(s_ptr<vertex>& v);
    face_id   add_face(s_ptr<vertex>& v0, s_ptr<vertex>& v1, s_ptr<vertex>& v2,
                       geometry::auto_vertex_normal_calculation avnc= geometry::auto_vertex_normal_calculation::OFF);

    // getter
    vertex_map       get_vertex_map() const         { return vertex_map_; }
    face_map         get_face_map() const           { return face_map_; }
    size_t           get_face_count() const         { return face_map_.size(); }
    size_t           get_vertex_count() const       { return vertex_map_.size(); }
    size_t           get_half_edge_count() const    { return half_edge_map_.size(); }
    s_ptr<face>      get_face(const face_id id)     { return face_map_[id]; }
    s_ptr<vertex>    get_vertex(const vertex_id id) { return vertex_map_[id]; }
    s_ptr<half_edge> get_half_edge(const s_ptr<vertex>& v0, const s_ptr<vertex>& v1);

    // setter
    void colorize_whole_mesh(const vec3& color);
  private:
    // returns false if the pair have not been registered to the map
    bool associate_half_edge_pair(const s_ptr<half_edge>& he);

    half_edge_map half_edge_map_;
    face_map      face_map_;
    vertex_map    vertex_map_;
};

} // namespace hnll::geometry