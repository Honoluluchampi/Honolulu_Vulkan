#pragma once
 
#include "hve_device.hpp"
 
namespace hve {
 
class HveBuffer {
  public:
    HveBuffer(
        HveDevice& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment = 1);
    ~HveBuffer();
  
    HveBuffer(const HveBuffer&) = delete;
    HveBuffer& operator=(const HveBuffer&) = delete;
  
    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();
  
    void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  
    // for multiple instances per buffer
    void writeToIndex(void* data, int index);
    VkResult flushIndex(int index);
    VkDescriptorBufferInfo descriptorInfoForIndex(int index);
    VkResult invalidateIndex(int index);
  
    VkBuffer getBuffer() const { return buffer_m; }
    void* getMappedMemory() const { return mapped_m; }
    uint32_t getInstanceCount() const { return instanceCount_m; }
    VkDeviceSize getInstanceSize() const { return instanceSize_m; }
    VkDeviceSize getAlignmentSize() const { return instanceSize_m; }
    VkBufferUsageFlags getUsageFlags() const { return usageFlags_m; }
    VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags_m; }
    VkDeviceSize getBufferSize() const { return bufferSize_m; }
  
  private:
    static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);
  
    HveDevice& hveDevice_m;
    void* mapped_m = nullptr;
    
    // separating buffer and its assigned memory enables programmer to manage memory manually
    VkBuffer buffer_m = VK_NULL_HANDLE;
    VkDeviceMemory memory_m = VK_NULL_HANDLE;
  
    VkDeviceSize bufferSize_m;
    uint32_t instanceCount_m;
    VkDeviceSize instanceSize_m;
    VkDeviceSize alignmentSize_m;
    VkBufferUsageFlags usageFlags_m;
    VkMemoryPropertyFlags memoryPropertyFlags_m;
};
 
}  // namespace Hve