#pragma once

// hnll
#include <graphics/renderer.hpp>

namespace hnll {
namespace gui {

#define GUI_RENDER_PASS_ID 1

class renderer : public hnll::graphics::renderer
{
  public:
    renderer(hnll::graphics::window& window, hnll::graphics::device& device, bool recreate_from_scratch);

    renderer(const renderer&) = delete;
    renderer& operator= (const renderer&) = delete;

    inline VkRenderPass get_render_pass()
    { return swap_chain_->get_render_pass(GUI_RENDER_PASS_ID); }

    void recreate_swap_chain() override;

  private:
    // specific for hie
    VkRenderPass create_render_pass();
    std::vector<VkFramebuffer> create_frame_buffers();
};

} // namespace gui
} // namespace hnll