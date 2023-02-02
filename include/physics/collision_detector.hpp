#pragma once

// hnll
#include <utils/common_using.hpp>

// std
#include <vector>
#include <memory>

namespace hnll {

// forward declaration
namespace game {
class rigid_component;
using rigid_component_id = unsigned;
}

namespace physics {

// forward declaration
class collision_info;

class collision_detector
{
  public:
    static std::vector<collision_info> intersection_test();

    static void add_rigid_component(const s_ptr<game::rigid_component>& comp) { rigid_components_.push_back(comp); }
  private:
    static std::vector<s_ptr<game::rigid_component>> rigid_components_;
};

}} // namespace hnll::physics