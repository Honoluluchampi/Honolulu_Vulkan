#pragma once

// std
#include <memory>
#include <vector>
#include <unordered_map>

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

class mesh_separation_helper
{
  public:
    mesh_separation_helper(const s_ptr<mesh_model>& model);
    vertex_map get_remaining_vertex_map() const { return remaining_vertex_map_; }
    face_map   get_remaining_face_map() const   { return remaining_face_map_; }
  private:
    s_ptr<mesh_model> model_;
    vertex_map remaining_vertex_map_;
    face_map   remaining_face_map_;
};

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
} // namespace hnll::geometry