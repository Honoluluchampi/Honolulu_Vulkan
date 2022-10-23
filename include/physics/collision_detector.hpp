#pragma once

// std
#include <vector>
#include <memory>

namespace hnll {

template <typename T> using u_ptr = std::unique_ptr<T>;
template <typename T> using s_ptr = std::shared_ptr<T>;

// forward declaration
namespace game {
class rigid_component;
using rigid_component_id = unsigned;
}

namespace physics {

class collision_detector
{
  public:
    static void add_rigid_component(const s_ptr<game::rigid_component>& comp) { rigid_components_.push_back(comp); }
  private:
    static std::vector<s_ptr<game::rigid_component>> rigid_components_;
};

}} // namespace hnll::physics