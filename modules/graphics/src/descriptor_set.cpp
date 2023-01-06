// hnll
#include <graphics/device.hpp>
#include <graphics/buffer.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <graphics/descriptor_set.hpp>

namespace hnll::graphics {

u_ptr<descriptor_set> descriptor_set::create(device& _device)
{ return std::make_unique<descriptor_set>(_device); }

descriptor_set& descriptor_set::create_pool(
  uint32_t max_sets, uint32_t desc_set_count, VkDescriptorType descriptor_type)
{
  pool_ = descriptor_pool::builder(device_)
    .set_max_sets(max_sets)
    .add_pool_size(descriptor_type, desc_set_count)
    .build();
  type_ = descriptor_type;
}

descriptor_set& descriptor_set::add_buffer(u_ptr<buffer>&& desc_buffer)
{ buffers_.emplace_back(std::move(desc_buffer)); }

descriptor_set& descriptor_set::add_layout(VkShaderStageFlagBits shader_stage)
{
  auto new_layout = descriptor_set_layout::builder(device_)
    .add_binding(0, type_, shader_stage)
    .build();

  layouts_.emplace_back(std::move(new_layout));
}

descriptor_set& descriptor_set::build_sets()
{
  if (buffers_.size() != layouts_.size())
    throw std::runtime_error("layouts count and buffers count should be same. (temporal feature");

  auto set_count = layouts_.size();
  sets_.resize(set_count);

  for (int i = 0; i < set_count; i++) {
    auto buffer_info = buffers_[i]->descriptor_info();
    descriptor_writer(*layouts_[i], *pool_)
      .write_buffer(0, &buffer_info)
      .build(sets_[i]);
  }
}

} // namespace hnll::graphics