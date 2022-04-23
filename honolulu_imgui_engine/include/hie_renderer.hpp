// TODO : integrate with hverenderer

#pragma once

#include <hve_device.hpp>
#include <hve_swap_chain.hpp>
#include <hve_window.hpp>

namespace hnll {

#define HIE_RENDER_PASS_ID 1

class HieRenderer : public HveRenderer
{
  public:
    HieRenderer(HveWindow& window, HveDevice& hveDevice, u_ptr<HveSwapChain>& upHveSwapChain);
    ~HieRenderer();

    HieRenderer(const HieRenderer&) = delete;
    HieRenderer& operator= (const HieRenderer&) = delete;

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    inline VkRenderPass getRenderPass()
    { return upHveSwapChain_->getRenderPass(HIE_RENDER_PASS_ID); }

    void recreateSwapChain() override;

  private:
    // specific for hie
    VkRenderPass createRenderPass();
    std::vector<VkFramebuffer> createFramebuffers();
    u_ptr<HveSwapChain>& upHveSwapChain_;
};

} // namespace hnll