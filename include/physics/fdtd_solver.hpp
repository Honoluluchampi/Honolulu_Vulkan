#pragma once

#include <memory>

namespace hnll {

template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

namespace physics {

class fdtd_solver
{

};

}} // namespace hnll::physics