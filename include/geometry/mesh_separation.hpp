#pragma once

// std
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <set>

namespace hnll {

// forward declaration
namespace graphics {
  struct mesh_builder;
  struct meshlet;
}
namespace geometry {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

// forward declaration
class mesh_model;
class vertex;
class face;
using vertex_id  = uint32_t;
using vertex_map = std::unordered_map<vertex_id, s_ptr<vertex>>;
using face_id    = uint32_t;
using face_map   = std::unordered_map<face_id, s_ptr<face>>;
using remaining_face_id_set = std::set<face_id>;

namespace mesh_separation {

constexpr int VERTEX_COUNT_PER_MESHLET = 64;
constexpr int PRIMITIVE_COUNT_PER_MESHLET = 126; // 2byte is

enum class solution {
  GREEDY,
  K_MEANS_BASED
};

enum class criterion {
  MINIMIZE_AABB,
  MINIMIZE_BOUNDING_SPHERE
};

std::vector<graphics::meshlet> separate(
  const s_ptr<mesh_model>& _model,
  const std::string& _model_name = "tmp",
  criterion _crtr = criterion::MINIMIZE_BOUNDING_SPHERE);

// meshlet cache file format
/*
 * model path
 * separation strategy (greedy...)
 * cost function type (sphere, aabb...)
 * meshlet count
 * meshlet id (0 ~ meshlet count) :
 *   vertex count
 *   meshlet count
 *   vertex indices array (split by , )
 *   triangle indices array (split by , )
 */

void write_meshlet_cache(
  const std::vector<graphics::meshlet>& _meshlets,
  const std::string& _filename,
  criterion _crtr);

// returns true if the cache exists
bool load_meshlet_cache(const std::string& _filename, std::vector<graphics::meshlet>& meshlets);

} // namespace mesh_separation

using criterion_map = std::unordered_map<mesh_separation::criterion, std::function<double()>>;
using solution_map  = std::unordered_map<mesh_separation::solution, std::function<s_ptr<mesh_model>()>>;

class mesh_separation_helper
{
  public:
    static s_ptr<mesh_separation_helper> create(
      const s_ptr<mesh_model>& model,
      const std::string& _model_name,
      mesh_separation::criterion criterion
      );
    explicit mesh_separation_helper(const s_ptr<mesh_model>& model);

    void compute_shape_diameter();

    std::vector<mesh_model> separate_using_sdf();

    // getter
    vertex_map  get_vertex_map()         const { return vertex_map_; }
    face_map    get_face_map()           const { return face_map_; }
    s_ptr<face> get_random_remaining_face();
    bool        all_face_is_registered() const { return remaining_face_id_set_.empty(); }
    bool        vertex_is_empty()        const { return vertex_map_.empty(); }
    std::string get_model_name()         const { return model_name_; }
    mesh_separation::criterion get_criterion() { return criterion_; }

    // setter
    void remove_face(face_id id) { remaining_face_id_set_.erase(id); }
    void update_adjoining_face_map(face_map& adjoining_face_map, const s_ptr<face>& fc);
    void set_criterion(mesh_separation::criterion crtr) { criterion_ = crtr; }
    void set_model_name(const std::string& _model_name) { model_name_ = _model_name; }
  private:
    s_ptr<mesh_model>     model_;

    s_ptr<vertex>         current_vertex_;
    vertex_map            vertex_map_;
    face_map              face_map_;
    remaining_face_id_set remaining_face_id_set_;

    static criterion_map  criterion_map_;
    static solution_map   solution_map_;

    // for cache
    mesh_separation::criterion criterion_;
    std::string                model_name_;
};

}} // namespace hnll::geometry