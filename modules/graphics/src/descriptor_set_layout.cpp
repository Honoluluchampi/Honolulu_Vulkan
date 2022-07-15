// hnll
#include <graphics/descriptor_set_layout.hpp>

// std
#include <cassert>
#include <stdexcept>

namespace hnll {

// *************** Descriptor Set Layout Builder *********************

HveDescriptorSetLayout::Builder &HveDescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) 
{
  assert(bindings_m.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptorType;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  bindings_m[binding] = layoutBinding;
  return *this;
}

std::unique_ptr<HveDescriptorSetLayout> HveDescriptorSetLayout::Builder::build() const 
{ return std::make_unique<HveDescriptorSetLayout>(hveDevice_m, bindings_m); }

// *************** Descriptor Set Layout *********************

HveDescriptorSetLayout::HveDescriptorSetLayout
  (device &get_device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : hveDevice_m{get_device}, bindings_m{bindings} 
{
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

  if (vkCreateDescriptorSetLayout(get_device.device(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout_m) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

HveDescriptorSetLayout::~HveDescriptorSetLayout()
{ vkDestroyDescriptorSetLayout(hveDevice_m.device(), descriptorSetLayout_m, nullptr); }

// *************** Descriptor Pool Builder *********************

HveDescriptorPool::Builder &HveDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count) 
{
  poolSizes_m.push_back({descriptorType, count});
  return *this;
}

HveDescriptorPool::Builder &HveDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) 
{
  poolFlags_m = flags;
  return *this;
}
HveDescriptorPool::Builder &HveDescriptorPool::Builder::setMaxSets(uint32_t count) 
{
  maxSets_m = count;
  return *this;
}

// build from its member
std::unique_ptr<HveDescriptorPool> HveDescriptorPool::Builder::build() const 
{ return std::make_unique<HveDescriptorPool>(hveDevice_m, maxSets_m, poolFlags_m, poolSizes_m); }

// *************** Descriptor Pool *********************

HveDescriptorPool::HveDescriptorPool(device &get_device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, 
const std::vector<VkDescriptorPoolSize> &poolSizes) : hveDevice_m{get_device} 
{
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;

  if (vkCreateDescriptorPool(hveDevice_m.device(), &descriptorPoolInfo, nullptr, &descriptorPool_m) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

HveDescriptorPool::~HveDescriptorPool() {
  vkDestroyDescriptorPool(hveDevice_m.device(), descriptorPool_m, nullptr);
}

bool HveDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const 
{
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool_m;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

  // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
  // a new pool whenever an old pool fills up. But this is beyond our current scope
  if (vkAllocateDescriptorSets(hveDevice_m.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
    return false;
  }
  return true;
}

void HveDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const 
{
  vkFreeDescriptorSets(
      hveDevice_m.device(),
      descriptorPool_m,
      static_cast<uint32_t>(descriptors.size()),
      descriptors.data());
}

void HveDescriptorPool::resetPool() 
{ vkResetDescriptorPool(hveDevice_m.device(), descriptorPool_m, 0); }

// *************** Descriptor Writer *********************

HveDescriptorWriter::HveDescriptorWriter(HveDescriptorSetLayout &setLayout, HveDescriptorPool &pool)
    : setLayout_m{setLayout}, pool_m{pool} {}

HveDescriptorWriter &HveDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo) 
{
  assert(setLayout_m.bindings_m.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = setLayout_m.bindings_m[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;

  writes_m.push_back(write);
  return *this;
}

HveDescriptorWriter &HveDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo) 
{
  assert(setLayout_m.bindings_m.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = setLayout_m.bindings_m[binding];

  assert(bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pImageInfo = imageInfo;
  write.descriptorCount = 1;

  writes_m.push_back(write);
  return *this;
}

bool HveDescriptorWriter::build(VkDescriptorSet &set) 
{
  bool success = pool_m.allocateDescriptor(setLayout_m.getDescriptorSetLayout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}

void HveDescriptorWriter::overwrite(VkDescriptorSet &set) 
{
  for (auto &write : writes_m) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(pool_m.hveDevice_m.device(), writes_m.size(), writes_m.data(), 0, nullptr);
}

}  // namespace hve