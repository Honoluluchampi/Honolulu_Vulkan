// hnll
#include <graphics/device.hpp>
#include <graphics/buffer.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <graphics/descriptor_set.hpp>

namespace hnll::graphics {

u_ptr<descriptor_set> descriptor_set::create(device& _device)
{ return std::make_unique<descriptor_set>(_device); }

descriptor_set::descriptor_set(device &_device) : device_(_device) {}

descriptor_set::~descriptor_set()
{ pool_->free_descriptors(sets_); }

descriptor_set& descriptor_set::create_pool(
  uint32_t max_sets, uint32_t desc_set_count, VkDescriptorType descriptor_type)
{
  pool_ = descriptor_pool::builder(device_)
    .set_max_sets(max_sets)
    .add_pool_size(descriptor_type, desc_set_count)
    .set_pool_flags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
    .build();
  type_ = descriptor_type;

  return *this;
}

descriptor_set& descriptor_set::add_buffer(u_ptr<buffer>&& desc_buffer)
{ buffers_.emplace_back(std::move(desc_buffer)); return *this; }

descriptor_set& descriptor_set::add_layout(VkShaderStageFlagBits shader_stage)
{
  layout_ = descriptor_set_layout::builder(device_)
    .add_binding(0, type_, shader_stage)
    .build();

  return *this;
}

descriptor_set& descriptor_set::build_sets()
{
  auto set_count = buffers_.size();
  sets_.resize(set_count);

  for (int i = 0; i < set_count; i++) {
    auto buffer_info = buffers_[i]->descriptor_info();
    descriptor_writer(*layout_, *pool_)
      .write_buffer(0, &buffer_info)
      .build(sets_[i]);
  }

  return *this;
}

// buffer update
void descriptor_set::write_to_buffer(size_t index, void *data) { buffers_[index]->write_to_buffer(data); }
void descriptor_set::flush_buffer(size_t index) { buffers_[index]->flush(); }

// getter
VkDescriptorSetLayout descriptor_set::get_layout() const
{ return layout_->get_descriptor_set_layout(); }

} // namespace hnll::graphics