// hnll
#include <graphics/descriptor_set_layout.hpp>

// std
#include <cassert>
#include <stdexcept>

namespace hnll {

// *************** Descriptor Set Layout builder *********************

descriptor_set_layout::builder &descriptor_set_layout::builder::add_binding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) 
{
  assert(bindings_.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptorType;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  bindings_[binding] = layoutBinding;
  return *this;
}

std::unique_ptr<descriptor_set_layout> descriptor_set_layout::builder::build() const 
{ return std::make_unique<descriptor_set_layout>(device_, bindings_); }

// *************** Descriptor Set Layout *********************

descriptor_set_layout::descriptor_set_layout
  (device &get_device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : device_{get_device}, bindings_{bindings} 
{
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

  if (vkCreateDescriptorSetLayout(get_device.device(), &descriptorSetLayoutInfo, nullptr, &descriptor_set_layout_) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

descriptor_set_layout::~descriptor_set_layout()
{ vkDestroyDescriptorSetLayout(device_.device(), descriptor_set_layout_, nullptr); }

// *************** Descriptor Pool builder *********************

descriptor_pool::builder &descriptor_pool::builder::add_pool_size(VkDescriptorType descriptorType, uint32_t count) 
{
  pool_sizes_.push_back({descriptorType, count});
  return *this;
}

descriptor_pool::builder &descriptor_pool::builder::set_pool_flags(VkDescriptorPoolCreateFlags flags) 
{
  pool_flags_ = flags;
  return *this;
}
descriptor_pool::builder &descriptor_pool::builder::set_max_sets(uint32_t count) 
{
  max_sets_ = count;
  return *this;
}

// build from its member
std::unique_ptr<descriptor_pool> descriptor_pool::builder::build() const 
{ return std::make_unique<descriptor_pool>(device_, max_sets_, pool_flags_, pool_sizes_); }

// *************** Descriptor Pool *********************

descriptor_pool::descriptor_pool(device &get_device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, 
const std::vector<VkDescriptorPoolSize> &poolSizes) : device_{get_device} 
{
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;

  if (vkCreateDescriptorPool(device_.device(), &descriptorPoolInfo, nullptr, &descriptor_pool_) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

descriptor_pool::~descriptor_pool() {
  vkDestroyDescriptorPool(device_.device(), descriptor_pool_, nullptr);
}

bool descriptor_pool::allocate_descriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const 
{
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptor_pool_;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

  // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
  // a new pool whenever an old pool fills up. But this is beyond our current scope
  if (vkAllocateDescriptorSets(device_.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
    return false;
  }
  return true;
}

void descriptor_pool::free_descriptors(std::vector<VkDescriptorSet> &descriptors) const 
{
  vkFreeDescriptorSets(
      device_.device(),
      descriptor_pool_,
      static_cast<uint32_t>(descriptors.size()),
      descriptors.data());
}

void descriptor_pool::reset_pool() 
{ vkResetDescriptorPool(device_.device(), descriptor_pool_, 0); }

// *************** Descriptor Writer *********************

descriptor_writer::descriptor_writer(descriptor_set_layout &setLayout, descriptor_pool &pool)
    : set_layout_{setLayout}, pool_{pool} {}

descriptor_writer &descriptor_writer::write_buffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo) 
{
  assert(set_layout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = set_layout_.bindings_[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;

  writes_.push_back(write);
  return *this;
}

descriptor_writer &descriptor_writer::write_image(uint32_t binding, VkDescriptorImageInfo *imageInfo) 
{
  assert(set_layout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = set_layout_.bindings_[binding];

  assert(bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pImageInfo = imageInfo;
  write.descriptorCount = 1;

  writes_.push_back(write);
  return *this;
}

bool descriptor_writer::build(VkDescriptorSet &set) 
{
  bool success = pool_.allocate_descriptor(set_layout_.get_descriptor_set_layout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}

void descriptor_writer::overwrite(VkDescriptorSet &set) 
{
  for (auto &write : writes_) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(pool_.device_.device(), writes_.size(), writes_.data(), 0, nullptr);
}

}  // namespace hve