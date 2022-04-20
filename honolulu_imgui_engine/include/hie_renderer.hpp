// TODO : integrate with hverenderer

#pragma once

#include <hve_device.hpp>
#include <hve_swap_chain.hpp>

namespace hnll {

class HieRenderer
{
  public:
    HieRenderer(HveDevice& hveDevice_, HveSwapChain& hveSwapChain);
    ~HieRenderer();

    HieRenderer(const HieRenderer&) = delete;
    HieRenderer& operator= (const HieRenderer&) = delete;

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    // getter
    VkCommandBuffer getCurrentCommandBuffer() const 
    { return commandBuffers_[currentFrameIndex_]; }
    VkCommandPool getCommandPool() const
    { return commandPool_; }

  private:
    void createCommandBuffers();
    // TODO : use HveDevice::commandPool_m;
    void createCommandPool();
    void freeCommandBuffers();
    void freeCommandPool();

    // shared vulkan objects
    VkDevice device_;
    HveSwapChain& hveSwapChain_;
    uint32_t graphicsFamilyIndex_;

    // specific vulkan objects
    std::vector<VkCommandBuffer> commandBuffers_;
    VkCommandPool commandPool_;

    uint32_t currentImageIndex_ = 0;
    int currentFrameIndex_ = 0;
    bool isFrameStarted_m = false;
};

} // namespace hnll