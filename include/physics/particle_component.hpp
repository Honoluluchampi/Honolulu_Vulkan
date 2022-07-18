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
    static s_ptr<particle_component> create_particle_component(game::actor& actor);
    ~particle_component() {}

    // disable copy-assignment
    particle_component(const particle_component&) = delete;
    particle_component& operator=(const particle_component&) = delete;

  private:
    particle_component();
    // TODO : consider to delete s_ptr<transform>
    s_ptr<hnll::utils::transform> transform_sp_;
    hnll::utils::transform position_ref_;
    glm::vec3 velocity_;
    float radius_ = 1.f;
};

} // namespace physics
} // namespace hnll