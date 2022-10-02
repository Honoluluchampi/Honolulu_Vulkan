#pragma once

// std
#include <memory>
#include <vector>

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

namespace hnll::geometry {

// forward declaration
class mesh_model;

namespace mesh_separation {

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