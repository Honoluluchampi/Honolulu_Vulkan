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

class HveRenderer
{
  public:

    HveRenderer(HveWindow& window, HveDevice& device);
    ~HveRenderer();

    HveRenderer(const HveRenderer &) = delete;
    HveRenderer &operator= (const HveRenderer &) = delete;

    // getter
    VkRenderPass getSwapChainRenderPass() const { return hveSwapChain_m->getRenderPass(); }
    float getAspectRatio() const { return hveSwapChain_m->extentAspectRatio(); }
    bool isFrameInProgress() const { return isFrameStarted_m; }
    
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
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

  private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    HveWindow& hveWindow_m;
    HveDevice& hveDevice_m;
    std::unique_ptr<HveSwapChain> hveSwapChain_m;
    std::vector<VkCommandBuffer> commandBuffers_m;

    uint32_t currentImageIndex_m = 0;
    int currentFrameIndex_m = 0; // [0, max_frames_in_flight]
    bool isFrameStarted_m = false;
};
} // namespace hve