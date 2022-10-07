#pragma once

// std
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <set>

namespace hnll::geometry {

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
using meshlet    = mesh_model;
using remaining_face_id_set = std::set<face_id>;

namespace mesh_separation {

constexpr int VERTEX_COUNT_PER_MESHLET = 64;
constexpr int PRIMITIVE_COUNT_PER_MESHLET = 126; // 2byte is

enum class solution {
  GREEDY,
  K_MEANS_BASED
};

enum class criterion {
  MINIMIZE_VARIATION
};

std::vector<s_ptr<mesh_model>> separate(const s_ptr<mesh_model>& model);

} // namespace mesh_separation

using criterion_map = std::unordered_map<mesh_separation::criterion, std::function<double()>>;
using solution_map  = std::unordered_map<mesh_separation::solution, std::function<s_ptr<meshlet>()>>;

class mesh_separation_helper
{
  public:
    static s_ptr<mesh_separation_helper> create(const s_ptr<mesh_model>& model);
    explicit mesh_separation_helper(const s_ptr<mesh_model>& model);

    // getter
    vertex_map  get_vertex_map() const         { return vertex_map_; }
    face_map    get_face_map() const           { return face_map_; }
    s_ptr<face> get_random_remaining_face();
    bool        all_face_is_registered() const { return remaining_face_id_set_.empty(); }
    bool        vertex_is_empty() const        { return vertex_map_.empty(); }

    // setter
    void remove_face(face_id id) { remaining_face_id_set_.erase(id); }
    void update_adjoining_face_map(face_map& adjoining_face_map, const s_ptr<face>& fc);

  private:
    s_ptr<mesh_model>     model_;
    s_ptr<vertex>         current_vertex_;
    vertex_map            vertex_map_;
    face_map              face_map_;
    remaining_face_id_set remaining_face_id_set_;
    static criterion_map  criterion_map_;
    static solution_map   solution_map_;
};

} // namespace hnll::geometry