#pragma once

// hnll
#include <utils/common_using.hpp>

// lib
#include <vulkan/vulkan.h>

namespace hnll::graphics {

class device;
class buffer;
class descriptor_pool;
class descriptor_set_layout;

class descriptor_set
{
  public:
    static u_ptr<descriptor_set> create(device& _device);
    descriptor_set(device& _device);
    ~descriptor_set();
    descriptor_set& create_pool(uint32_t max_sets, uint32_t desc_set_count, VkDescriptorType descriptor_type);
    descriptor_set& add_buffer(u_ptr<buffer>&& desc_buffer);
    descriptor_set& add_layout(VkShaderStageFlagBits shader_stage);
    descriptor_set& build_sets();
  private:
    device& device_;
    u_ptr<descriptor_pool>       pool_;
    std::vector<u_ptr<descriptor_set_layout>> layouts_;
    std::vector<u_ptr<buffer>>   buffers_;
    std::vector<VkDescriptorSet> sets_;
    VkDescriptorType             type_;
};

} // namespace hnll::graphics