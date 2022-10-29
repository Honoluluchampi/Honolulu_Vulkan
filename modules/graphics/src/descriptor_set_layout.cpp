// hnll
#include <graphics/descriptor_set_layout.hpp>

// std
#include <cassert>
#include <stdexcept>

namespace hnll::graphics {
// *************** Descriptor Set Layout builder *********************

descriptor_set_layout::builder &descriptor_set_layout::builder::add_binding(
    uint32_t binding,
    VkDescriptorType descriptor_type,
    VkShaderStageFlags stage_flags,
    uint32_t count) 
{
  assert(bindings_.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layout_binding{};
  layout_binding.binding = binding;
  layout_binding.descriptorType = descriptor_type;
  layout_binding.descriptorCount = count;
  layout_binding.stageFlags = stage_flags;
  bindings_[binding] = layout_binding;
  return *this;
}

std::unique_ptr<descriptor_set_layout> descriptor_set_layout::builder::build() const 
{ return std::make_unique<descriptor_set_layout>(device_, bindings_); }

// *************** Descriptor Set Layout *********************

descriptor_set_layout::descriptor_set_layout
  (device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : device_{device}, bindings_{bindings} 
{
  std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
  for (auto kv : bindings) {
    set_layout_bindings.push_back(kv.second);
  }

  VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
  descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
  descriptor_set_layout_info.pBindings = set_layout_bindings.data();

  if (vkCreateDescriptorSetLayout(device.get_device(), &descriptor_set_layout_info, nullptr, &descriptor_set_layout_) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

descriptor_set_layout::~descriptor_set_layout()
{ vkDestroyDescriptorSetLayout(device_.get_device(), descriptor_set_layout_, nullptr); }

// *************** Descriptor Pool builder *********************

descriptor_pool::builder &descriptor_pool::builder::add_pool_size(VkDescriptorType descriptor_type, uint32_t count) 
{
  pool_sizes_.push_back({descriptor_type, count});
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

descriptor_pool::descriptor_pool(device &device, uint32_t max_sets, VkDescriptorPoolCreateFlags pool_flags, 
const std::vector<VkDescriptorPoolSize> &pool_sizes) : device_{device} 
{
  VkDescriptorPoolCreateInfo descriptor_pool_info{};
  descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  descriptor_pool_info.pPoolSizes = pool_sizes.data();
  descriptor_pool_info.maxSets = max_sets;
  descriptor_pool_info.flags = pool_flags;

  if (vkCreateDescriptorPool(device_.get_device(), &descriptor_pool_info, nullptr, &descriptor_pool_) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

descriptor_pool::~descriptor_pool() {
  vkDestroyDescriptorPool(device_.get_device(), descriptor_pool_, nullptr);
}

bool descriptor_pool::allocate_descriptor(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet &descriptor) const 
{
  VkDescriptorSetAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = descriptor_pool_;
  alloc_info.pSetLayouts = &descriptor_set_layout;
  alloc_info.descriptorSetCount = 1;

  // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
  // a new pool whenever an old pool fills up. But this is beyond our current scope
  if (vkAllocateDescriptorSets(device_.get_device(), &alloc_info, &descriptor) != VK_SUCCESS) {
    return false;
  }
  return true;
}

void descriptor_pool::free_descriptors(std::vector<VkDescriptorSet> &descriptors) const 
{
  vkFreeDescriptorSets(
      device_.get_device(),
      descriptor_pool_,
      static_cast<uint32_t>(descriptors.size()),
      descriptors.data());
}

void descriptor_pool::reset_pool() 
{ vkResetDescriptorPool(device_.get_device(), descriptor_pool_, 0); }

// *************** Descriptor Writer *********************

descriptor_writer::descriptor_writer(descriptor_set_layout &set_layout, descriptor_pool &pool)
    : set_layout_{set_layout}, pool_{pool} {}

descriptor_writer &descriptor_writer::write_buffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info) 
{
  assert(set_layout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

  auto &binding_description = set_layout_.bindings_[binding];

  assert(
      binding_description.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = binding_description.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = buffer_info;
  write.descriptorCount = 1;

  writes_.push_back(write);
  return *this;
}

descriptor_writer &descriptor_writer::write_image(uint32_t binding, VkDescriptorImageInfo *image_info) 
{
  assert(set_layout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

  auto &binding_description = set_layout_.bindings_[binding];

  assert(binding_description.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = binding_description.descriptorType;
  write.dstBinding = binding;
  write.pImageInfo = image_info;
  write.descriptorCount = 1;

  writes_.push_back(write);
  return *this;
}

descriptor_writer& descriptor_writer::write_acceleration_structure(uint32_t binding, void* as_info)
{
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = as_info;
  write.dstBinding = binding;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

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
  vkUpdateDescriptorSets(pool_.device_.get_device(), writes_.size(), writes_.data(), 0, nullptr);
}

} // namespace hnll::graphics