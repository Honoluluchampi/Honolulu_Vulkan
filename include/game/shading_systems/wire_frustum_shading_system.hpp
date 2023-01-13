#pragma once

// hnll
#include <game/shading_system.hpp>

namespace hnll {
namespace game {

class wire_frustum_shading_system : public shading_system
{
  public:
    static u_ptr<wire_frustum_shading_system> create(graphics::device& device);
    wire_frustum_shading_system(graphics::device& device);
    void render(const utils::frame_info& frame_info) override;
};
}} // namespace hnll::game