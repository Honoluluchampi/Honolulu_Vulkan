#pragma once

// hnll
#include <game/shading_system.hpp>
#include <graphics/descriptor_set.hpp>
#include <graphics/buffer.hpp>

namespace hnll::game {

class frame_anim_meshlet_shading_system : public shading_system
{
  public:
    static u_ptr<frame_anim_meshlet_shading_system> create(graphics::device& device);
    explicit frame_anim_meshlet_shading_system(graphics::device& device);
    void render(const utils::frame_info& frame_info) override;
  private:
    void setup_task_desc();

    std::vector<u_ptr<graphics::buffer>> task_desc_buffers_;
    u_ptr<graphics::descriptor_set>      task_desc_sets_;
};
} // namespace hnll::game