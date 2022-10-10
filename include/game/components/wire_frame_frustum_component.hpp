#pragma once

// hnll
#include <game/components/renderable_component.hpp>

namespace hnll {

// forward declaration
namespace geometry { class perspective_frustum; }

namespace game {

class wire_frame_frustum_component : public renderable_component
{
  public:
    template <Actor A>
    static s_ptr<wire_frame_frustum_component> create(s_ptr<A>& owner_sp, geometry::perspective_frustum& frustum);
    ~wire_frame_frustum_component() = default;

  private:

};

}} // namespace hnll::game