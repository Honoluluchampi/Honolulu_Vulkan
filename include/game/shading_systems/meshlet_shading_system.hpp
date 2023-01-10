#pragma once

// hnll
#include <game/shading_system.hpp>
#include <graphics/descriptor_set.hpp>
#include <graphics/buffer.hpp>

namespace hnll::game {

class meshlet_shading_system : public shading_system
{
  public:
    static u_ptr<meshlet_shading_system> create(graphics::device& device);
    explicit meshlet_shading_system(graphics::device& device);
    void render(const utils::frame_info& frame_info) override;
  private:
    void setup_task_desc();

    u_ptr<graphics::descriptor_set>      task_desc_sets_;
};

} // namespace hnll::game