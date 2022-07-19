#pragma once

// hnll
#include <utils/utils.hpp>

namespace hnll {

// forward declaration
namespace game { class actor; }

namespace physics {

class particle_component
{
  public:
    static s_ptr<particle_component> create(game::actor& actor);
    particle_component(s_ptr<hnll::utils::transform>&& transform_sp);
    ~particle_component() {}

    // disable copy-assignment
    particle_component(const particle_component&) = delete;
    particle_component& operator=(const particle_component&) = delete;

  private:
    // TODO : consider to delete s_ptr<transform>
    s_ptr<hnll::utils::transform> transform_sp_;
    glm::vec3& position_ref_;
    glm::vec3 velocity_;
    float radius_ = 1.f;
};

} // namespace physics
} // namespace hnll