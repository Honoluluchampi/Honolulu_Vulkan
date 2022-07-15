/*
 * Encapsulates a graphics buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

// hnll
#include <graphics/buffer.hpp>
 
// std
#include <cassert>
#include <cstring>
 
namespace hnll {
 
/**
 * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
 *
 * @param instanceSize The size of an instance
 * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
 * minUniformBufferOffsetAlignment)
 *
 * @return VkResult of the buffer mapping call
 */
VkDeviceSize HveBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) 
{
  if (minOffsetAlignment > 0) {
    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
  }
  return instanceSize;
}
 
HveBuffer::HveBuffer(
    device &device,
    VkDeviceSize instanceSize,
    uint32_t instanceCount,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment)
    : hveDevice_m{device},
      instanceSize_m{instanceSize},
      instanceCount_m{instanceCount},
      usageFlags_m{usageFlags},
      memoryPropertyFlags_m{memoryPropertyFlags} 
{
  alignmentSize_m = getAlignment(instanceSize, minOffsetAlignment);
  bufferSize_m = alignmentSize_m * instanceCount;
  hveDevice_m.createBuffer(bufferSize_m, usageFlags, memoryPropertyFlags, buffer_m, memory_m);
}
 
HveBuffer::~HveBuffer() 
{
  unmap();
  vkDestroyBuffer(hveDevice_m.device(), buffer_m, nullptr);
  vkFreeMemory(hveDevice_m.device(), memory_m, nullptr);
}
 
/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */
VkResult HveBuffer::map(VkDeviceSize size, VkDeviceSize offset) 
{
  assert(buffer_m && memory_m && "Called map on buffer before create");
  return vkMapMemory(hveDevice_m.device(), memory_m, offset, size, 0, &mapped_m);
}
 
/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void HveBuffer::unmap() 
{
  if (mapped_m) {
    vkUnmapMemory(hveDevice_m.device(), memory_m);
    mapped_m = nullptr;
  }
}
 
/**
 * Copies the specified data to the mapped buffer. Default value writes whole buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
 * range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
void HveBuffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) 
{
  assert(mapped_m && "Cannot copy to unmapped buffer");
 
  if (size == VK_WHOLE_SIZE) {
    memcpy(mapped_m, data, bufferSize_m);
  } else {
    char *memOffset = (char *)mapped_m;
    memOffset += offset;
    memcpy(memOffset, data, size);
  }
}
 
/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
VkResult HveBuffer::flush(VkDeviceSize size, VkDeviceSize offset) 
{
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = memory_m;
  mappedRange.offset = offset;
  mappedRange.size = size;
  return vkFlushMappedMemoryRanges(hveDevice_m.device(), 1, &mappedRange);
}
 
/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
VkResult HveBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) 
{
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = memory_m;
  mappedRange.offset = offset;
  mappedRange.size = size;
  return vkInvalidateMappedMemoryRanges(hveDevice_m.device(), 1, &mappedRange);
}
 
/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
VkDescriptorBufferInfo HveBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) 
{ return VkDescriptorBufferInfo{buffer_m, offset, size,}; }
 
/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void HveBuffer::writeToIndex(void *data, int index) 
{ writeToBuffer(data, instanceSize_m, index * alignmentSize_m); }
 
/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
 *
 * @param index Used in offset calculation
 *
 */
VkResult HveBuffer::flushIndex(int index) 
{ return flush(alignmentSize_m, index * alignmentSize_m); }
 
/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
VkDescriptorBufferInfo HveBuffer::descriptorInfoForIndex(int index) 
{ return descriptorInfo(alignmentSize_m, index * alignmentSize_m); }
 
/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param index Specifies the region to invalidate: index * alignmentSize
 *
 * @return VkResult of the invalidate call
 */
VkResult HveBuffer::invalidateIndex(int index)
{ return invalidate(alignmentSize_m, index * alignmentSize_m); }
 
}  // namespace engine