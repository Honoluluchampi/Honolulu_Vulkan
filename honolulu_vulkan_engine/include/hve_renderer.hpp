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

// define ~_RENDER_PASS_ID in renderer class
// to specify indices of multiple render pass and frame buffer
#define HVE_RENDER_PASS_ID 0

class HveRenderer
{
  public:

    HveRenderer(HveWindow& window, HveDevice& device);
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

    virtual void recreateSwapChainDependencies(){}

  private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

  protected:    
    inline void isLastRenderer()
    { isLastRenderer_m = true; }

    HveWindow& hveWindow_m;
    HveDevice& hveDevice_m;
    s_ptr<HveSwapChain> hveSwapChain_m;
    std::vector<VkCommandBuffer> commandBuffers_m;

    uint32_t currentImageIndex_m = 0;
    int currentFrameIndex_m = 0; // [0, max_frames_in_flight]
    bool isFrameStarted_m = false;

// to specify renderer by which command is submitted
#ifndef __IMGUI_DISABLED
    bool isLastRenderer_m = false;
#else 
    bool isLastRenderer_m = true;
#endif

};
} // namespace hve