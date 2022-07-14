#pragma once

// hnll
#include <graphics/renderer.hpp>

namespace hnll {

#define HIE_RENDER_PASS_ID 1

class HieRenderer : public HveRenderer
{
  public:
    HieRenderer(HveWindow& window, HveDevice& hveDevice, bool recreateFromScratch);

    HieRenderer(const HieRenderer&) = delete;
    HieRenderer& operator= (const HieRenderer&) = delete;

    inline VkRenderPass getRenderPass()
    { return hveSwapChain_m->getRenderPass(HIE_RENDER_PASS_ID); }

    void recreateSwapChain() override;

  private:
    // specific for hie
    VkRenderPass createRenderPass();
    std::vector<VkFramebuffer> createFramebuffers();
};

} // namespace hnll