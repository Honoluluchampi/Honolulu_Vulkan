#pragma once

// hnll
#include <utils/utils.hpp>
#include <physics/bounding_volume.hpp>

namespace hnll::physics {

class rigid_component
{
  public:
    rigid_component(bounding_volume& bv) : bounding_volume_(bv){}

    // getter
    const bounding_volume&  get_bounding_volume() const { return bounding_volume_; }
    const utils::transform& get_transform() const { return *transform_sp_; }

  private:
    s_ptr<hnll::utils::transform> transform_sp_;
    bounding_volume bounding_volume_;
};

} // namespace hnll::physics