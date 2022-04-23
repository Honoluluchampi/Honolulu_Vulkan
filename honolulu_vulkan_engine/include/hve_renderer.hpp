// resposible for swap chain, command buffers, drawing frame

#pragma once

#include <hve_window.hpp>
#include <hve_device.hpp>
#include <hve_swap_chain.hpp>

// std
#include <memory>
#include <vector>
#include <cassert>

namespace hnll {

// TODO : s_ptr<HveSwapChain> doesnt work correctly

// define ~_RENDER_PASS_ID in renderer class
// to specify indices of multiple render pass and frame buffer
#define HVE_RENDER_PASS_ID 0

template <class S> using s_ptr = std::shared_ptr<S>;
template <class U> using u_ptr = std::unique_ptr<U>;

class HveRenderer
{
  public:

    HveRenderer(HveWindow& window, HveDevice& device, s_ptr<HveSwapChain> swapChain = nullptr);
    ~HveRenderer();

    HveRenderer(const HveRenderer &) = delete;
    HveRenderer &operator= (const HveRenderer &) = delete;

    // getter
#ifdef __IMGUI_DISABLED
    inline VkRenderPass getSwapChainRenderPass() const { return hveSwapChain_m->getRenderPass(); }
#else
    inline VkRenderPass getSwapChainRenderPass(int renderPassId) const 
    { return hveSwapChain_m->getRenderPass(renderPassId); }
#endif 

    inline float getAspectRatio() const { return hveSwapChain_m->extentAspectRatio(); }
    inline bool isFrameInProgress() const { return isFrameStarted_m; }
    inline HveSwapChain& hveSwapChain() const { return *hveSwapChain_m; }
    inline const s_ptr<HveSwapChain>& spHveSwapChain() const { return hveSwapChain_m; }
    inline VkCommandPool getCommandPool() const { return hveDevice_m.getCommandPool(); }
    
    VkCommandBuffer getCurrentCommandBuffer() const 
    {
      assert(isFrameStarted_m && "Cannot get command buffer when frame not in progress");
      return commandBuffers_m[currentFrameIndex_m];
    }
    int getFrameIndex() const 
    {
      assert(isFrameStarted_m && "Cannot get frame when frame not in progress");
      return currentFrameIndex_m;
    }

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer, int renderPassId);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

#ifndef __IMGUI_DISABLED
    void submitCommandBuffers();
#endif

    virtual void recreateSwapChain();

    inline void setNextRenderer(HveRenderer* renderer)
    { nextRenderer_ = renderer; }

    const bool isLastRenderer() const 
    { return !nextRenderer_; }

    static bool swapChainRecreated_m;

  private:
    void createCommandBuffers();
    void freeCommandBuffers();

  protected:    
    HveWindow& hveWindow_m;
    HveDevice& hveDevice_m;
    std::vector<VkCommandBuffer> commandBuffers_m;

    static uint32_t currentImageIndex_m;
    static int currentFrameIndex_m; // [0, max_frames_in_flight]
    static bool isFrameStarted_m;

#ifndef __IMGUI_DISABLED
    // store multiple renderers' command buffers
    static std::vector<VkCommandBuffer> submittingCommandBuffers_m;
#endif

    // TODO : use smart pointer
    HveRenderer* nextRenderer_ = nullptr;

    // derived renderer's should take ref's of this
    s_ptr<HveSwapChain> hveSwapChain_m;
};
} // namespace hve