// hnll
#include <graphics/meshlet_model.hpp>
#include <graphics/buffer.hpp>

namespace hnll::graphics {

meshlet_model::meshlet_model(std::vector<vertex> &&raw_vertices, std::vector<meshlet<>> &&meshlets)
{
  raw_vertices_ = std::move(raw_vertices);
  meshlets_     = std::move(meshlets);
}

u_ptr<meshlet_model> meshlet_model::create(
  device& _device,
  std::vector<vertex>&& _raw_vertices, std::vector<meshlet<>>&& _meshlets)
{
  auto ret = std::make_unique<meshlet_model>(
    std::move(_raw_vertices),
    std::move(_meshlets)
  );

  ret->create_vertex_buffer(_device);
  ret->create_meshlet_buffer(_device);

  return ret;
}

void meshlet_model::create_vertex_buffer(device& _device)
{
  vertex_buffer_ = graphics::buffer::create_with_staging(
    _device,
    sizeof(vertex),
    raw_vertices_.size(),
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    raw_vertices_.data()
  );
}
void meshlet_model::create_meshlet_buffer(device& _device)
{
  vertex_buffer_ = graphics::buffer::create_with_staging(
    _device,
    sizeof(meshlet<>),
    meshlets_.size(),
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    meshlets_.data()
  );
}

// getter
const buffer& meshlet_model::get_vertex_buffer()  const
{ return *vertex_buffer_; }

const buffer& meshlet_model::get_meshlet_buffer() const
{ return *meshlet_buffer_; }

} // namespace hnll::graphics