#pragma once

// std
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

namespace hnll::geometry {

// forward declaration
class mesh_model;
class vertex;
class face;
using vertex_id  = uint32_t;
using vertex_map = std::unordered_map<vertex_id, s_ptr<vertex>>;
using face_id    = uint32_t;
using face_map   = std::unordered_map<face_id, s_ptr<face>>;
using meshlet    = mesh_model;

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
} // namespace mesh_separation

using criterion_map = std::unordered_map<mesh_separation::criterion, std::function<double()>>;
using solution_map  = std::unordered_map<mesh_separation::solution, std::function<s_ptr<meshlet>()>>;

class mesh_separation_helper
{
  public:
    static s_ptr<mesh_separation_helper> create(const s_ptr<mesh_model>& model);
    explicit mesh_separation_helper(const s_ptr<mesh_model>& model);
    std::vector<s_ptr<mesh_model>> separate(const s_ptr<mesh_model>& model);

    // getter
    vertex_map  get_remaining_vertex_map() const { return remaining_vertex_map_; }
    face_map    get_remaining_face_map() const   { return remaining_face_map_; }
    s_ptr<face> get_random_face();
    bool        face_is_empty() const            { return remaining_face_map_.empty(); }
    bool        vertex_is_empty() const          { return remaining_vertex_map_.empty(); }
  private:
    s_ptr<mesh_model>    model_;
    s_ptr<vertex>        current_vertex_;
    vertex_map           remaining_vertex_map_;
    face_map             remaining_face_map_;
    static criterion_map criterion_map_;
    static solution_map  solution_map_;
};

} // namespace hnll::geometry