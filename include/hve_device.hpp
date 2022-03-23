#pragma once

#include <hve_window.hpp>

// std
#include <string>
#include <vector>
#include <optional>

namespace hve {

// non-member function
struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities_m;
  std::vector<VkSurfaceFormatKHR> formats_m;
  std::vector<VkPresentModeKHR> presentModes_m;
};

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily_m = std::nullopt;
  std::optional<uint32_t> presentFamily_m = std::nullopt;
  inline bool isComplete() { return (graphicsFamily_m != std::nullopt) && (presentFamily_m != std::nullopt); };
};

class HveDevice 
{
  public:
    #ifdef NDEBUG
      const bool enableValidationLayers = false;
    #else
      const bool enableValidationLayers = true;
    #endif

    HveDevice(HveWindow &window);
    ~HveDevice();

    // Not copyable or movable
    HveDevice(const HveDevice &) = delete;
    void operator=(const HveDevice &) = delete;
    HveDevice(HveDevice &&) = delete;
    HveDevice &operator=(HveDevice &&) = delete;

    // getter
    VkCommandPool getCommandPool() { return commandPool_m; }
    VkDevice device() { return device_m; }
    VkSurfaceKHR surface() { return surface_m; }
    VkQueue graphicsQueue() { return graphicsQueue_m; }
    VkQueue presentQueue() { return presentQueue_m; }

    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice_m); }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice_m); }
    VkFormat findSupportedFormat(
        const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    // Buffer Helper Functions
    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer &buffer,
        VkDeviceMemory &bufferMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(
        VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

    void createImageWithInfo(
        const VkImageCreateInfo &imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory);

    VkPhysicalDeviceProperties properties;

  private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    // helper functions
    bool isDeviceSuitable(VkPhysicalDevice device);
    std::vector<const char *> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void hasGflwRequiredInstanceExtensions();
    // check for swap chain extension
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkInstance instance_m;
    VkDebugUtilsMessengerEXT debugMessenger_m;
    // implicitly destroyed when vkInstance is destroyed
    VkPhysicalDevice physicalDevice_m = VK_NULL_HANDLE;
    HveWindow &window_m;
    // Command pools manage the memory that is used to store the buffers 
    // and command buffers are allocated from them.
    VkCommandPool commandPool_m;

    VkDevice device_m;
    VkSurfaceKHR surface_m;
    VkQueue graphicsQueue_m;
    VkQueue presentQueue_m;

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

}  // namespace lve