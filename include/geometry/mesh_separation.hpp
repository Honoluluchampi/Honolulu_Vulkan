#pragma once

// std
#include <memory>
#include <vector>

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

namespace hnll::geometry {

// forward declaration
class mesh_model;

std::vector<s_ptr<mesh_model>> separate_mesh(const s_ptr<mesh_model>& model);

} // namespace hnll::geometry