// hnll
#include <graphics/meshlet_model.hpp>
#include <graphics/buffer.hpp>

namespace hnll::graphics {

meshlet_model::meshlet_model(std::vector<vertex> &&raw_vertices, std::vector<meshlet<>> &&meshlets)
{
  raw_vertices_ = std::move(raw_vertices);
  meshlets_     = std::move(meshlets);
}
} // namespace hnll::graphics