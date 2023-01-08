#pragma once

// hnll
#include <game/shading_system.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <graphics/buffer.hpp>

namespace hnll::game {

class meshlet_model_shading_system : public shading_system
{
  public:
    static u_ptr<meshlet_model_shading_system> create(graphics::device& device);
    explicit meshlet_model_shading_system(graphics::device& device);
    void render(const utils::frame_info& frame_info) override;
  private:
    void setup_task_desc();

    u_ptr<graphics::descriptor_pool> task_desc_pool_;
    std::vector<u_ptr<graphics::buffer>> task_desc_buffers_;
    u_ptr<graphics::descriptor_set_layout> task_desc_layout_;
    std::vector<VkDescriptorSet> task_desc_sets_;


};

} // namespace hnll::game