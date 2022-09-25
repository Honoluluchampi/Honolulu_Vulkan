#pragma once

// hnll
#include <game/component.hpp>
#include <utils/utils.hpp>
#include <physics/bounding_volume.hpp>

// forward declaration
namespace hnll::game { class mesh_component; }

namespace hnll::physics {

class rigid_component : public hnll::game::component
{
  public:
    static s_ptr<rigid_component> create_with_aabb(const s_ptr<hnll::game::mesh_component>& mesh_component_sp);

    rigid_component(bounding_volume& bv) : bounding_volume_(bv){}

    void specific_add_process(hnll::game::actor& owner) override;
    // getter
    const bounding_volume&        get_bounding_volume() const { return bounding_volume_; }
    const utils::transform&       get_transform() const { return *transform_sp_; }
    const s_ptr<utils::transform> get_transform_sp() const { return transform_sp_; }

    // setter
    void set_transform(s_ptr<utils::transform>& transform_sp) { transform_sp_ = transform_sp; }
  private:
    s_ptr<hnll::utils::transform> transform_sp_;
    bounding_volume bounding_volume_;
};

} // namespace hnll::physics