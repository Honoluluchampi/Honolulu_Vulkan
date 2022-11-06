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
 
namespace hnll::graphics {

VkDeviceSize buffer::get_alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment) 
{
  if (min_offset_alignment > 0) {
    return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
  }
  return instance_size;
}
 
buffer::buffer(
    device &device,
    VkDeviceSize instance_size,
    uint32_t instance_count,
    VkBufferUsageFlags usage_flags,
    VkMemoryPropertyFlags memory_property_flags,
    VkDeviceSize min_offset_alignment)
    : device_{device},
      instance_size_{instance_size},
      instance_count_{instance_count},
      usage_flags_{usage_flags},
      memory_property_flags_{memory_property_flags} 
{
  alignment_size_ = get_alignment(instance_size, min_offset_alignment);
  buffer_size_ = alignment_size_ * instance_count;
  device_.create_buffer(buffer_size_, usage_flags, memory_property_flags, buffer_, memory_);
}
 
buffer::~buffer() 
{
  unmap();
  vkDestroyBuffer(device_.get_device(), buffer_, nullptr);
  vkFreeMemory(device_.get_device(), memory_, nullptr);
}

u_ptr<buffer> buffer::create(
  hnll::graphics::device &device,
  VkDeviceSize instance_size,
  uint32_t instance_count,
  VkBufferUsageFlags usage_flags,
  VkMemoryPropertyFlags memory_property_flags,
  void* writing_data,
  VkDeviceSize min_offset_alignment)
{
  auto ret = std::make_unique<buffer>(device, instance_size, instance_count, usage_flags, memory_property_flags, min_offset_alignment);

  ret->map();
  ret->write_to_buffer(writing_data);

  return ret;
}

u_ptr<buffer> buffer::create_with_staging(
  hnll::graphics::device &device,
  VkDeviceSize instance_size,
  uint32_t instance_count,
  VkBufferUsageFlags usage_flags,
  VkMemoryPropertyFlags memory_property_flags,
  void* writing_data,
  VkDeviceSize min_offset_alignment)
{
  auto staging = std::make_unique<buffer>(
    device,
    instance_size,
    instance_count,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    min_offset_alignment
  );

  staging->map();
  staging->write_to_buffer(writing_data);

  auto ret = std::make_unique<graphics::buffer>(
    device,
    instance_size,
    instance_count,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage_flags,
    memory_property_flags,
    min_offset_alignment
  );

  VkDeviceSize buffer_size = instance_size * instance_count;
  device.copy_buffer(staging->get_buffer(), ret->get_buffer(), buffer_size);

  return ret;
}

VkResult buffer::map(VkDeviceSize size, VkDeviceSize offset) 
{
  assert(buffer_ && memory_ && "Called map on buffer before create");
  return vkMapMemory(device_.get_device(), memory_, offset, size, 0, &mapped_);
}
 
void buffer::unmap() 
{
  if (mapped_) {
    vkUnmapMemory(device_.get_device(), memory_);
    mapped_ = nullptr;
  }
}

void buffer::write_to_buffer(void *data, VkDeviceSize size, VkDeviceSize offset) 
{
  assert(mapped_ && "Cannot copy to unmapped buffer");
 
  if (size == VK_WHOLE_SIZE) {
    memcpy(mapped_, data, buffer_size_);
  } else {
    char *memory_offset = (char *)mapped_;
    memory_offset += offset;
    memcpy(memory_offset, data, size);
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
VkResult buffer::flush(VkDeviceSize size, VkDeviceSize offset) 
{
  VkMappedMemoryRange mapped_range = {};
  mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mapped_range.memory = memory_;
  mapped_range.offset = offset;
  mapped_range.size = size;
  return vkFlushMappedMemoryRanges(device_.get_device(), 1, &mapped_range);
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
VkResult buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) 
{
  VkMappedMemoryRange mapped_range = {};
  mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mapped_range.memory = memory_;
  mapped_range.offset = offset;
  mapped_range.size = size;
  return vkInvalidateMappedMemoryRanges(device_.get_device(), 1, &mapped_range);
}
 
/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
VkDescriptorBufferInfo buffer::descriptor_info(VkDeviceSize size, VkDeviceSize offset) const
{ return VkDescriptorBufferInfo{buffer_, offset, size,}; }
 
/**
 * Copies "instance_size" bytes of data to the mapped buffer at an offset of index * alignmentSize
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void buffer::write_to_index(void *data, int index) 
{ write_to_buffer(data, instance_size_, index * alignment_size_); }
 
/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
 *
 * @param index Used in offset calculation
 *
 */
VkResult buffer::flush_index(int index) 
{ return flush(alignment_size_, index * alignment_size_); }
 
/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
VkDescriptorBufferInfo buffer::descriptor_info_for_index(int index) 
{ return descriptor_info(alignment_size_, index * alignment_size_); }
 
/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param index Specifies the region to invalidate: index * alignmentSize
 *
 * @return VkResult of the invalidate call
 */
VkResult buffer::invalidate_index(int index)
{ return invalidate(alignment_size_, index * alignment_size_); }
 
} // namespace hnll::graphics