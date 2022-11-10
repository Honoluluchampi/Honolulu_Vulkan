// hnll
#include <graphics/mesh_model.hpp>
#include <graphics/utils.hpp>
#include <geometry/mesh_model.hpp>
#include <geometry/half_edge.hpp>

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cstring>
#include <iostream>
#include <unordered_map>

namespace std {

template <>
struct hash<hnll::graphics::vertex>
{
  size_t operator() (hnll::graphics::vertex const &vertex) const
  {
    // stores final hash value
    size_t seed = 0;
    hnll::graphics::hash_combine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
    return seed;
  }
};

} // namespace std

namespace hnll::graphics {

mesh_model::mesh_model(device& device, const mesh_builder &builder) : device_{device}
{
  create_vertex_buffers(builder.vertices);
  create_index_buffers(builder.indices);

  vertex_list_ = std::move(builder.vertices);
}

mesh_model::~mesh_model()
{
  // buffers will be freed in dtor of hnll::graphics::buffer
}

std::shared_ptr<mesh_model> mesh_model::create_model_from_file(device &device, const std::string &filename)
{
  mesh_builder builder;
  builder.load_model(filename);
  std::cout << filename << " vertex count: " << builder.vertices.size() << "\n";
  return std::make_shared<mesh_model>(device, builder);
}

void mesh_model::create_vertex_buffers(const std::vector<vertex> &vertices)
{
  // vertexCount must be larger than 3 (triangle) 
  // use a host visible buffer as temporary buffer, use a device local buffer as actual vertex buffer
  vertex_count_ = static_cast<uint32_t>(vertices.size());
  VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;
  uint32_t vertex_size = sizeof(vertices[0]);

  // staging buffer creation
  buffer staging_buffer {
    device_,
    vertex_size, // for calculating alignment
    vertex_count_, // same as above
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // usage
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT // property
  };
  // mapping the data to the buffer
  staging_buffer.map();
  staging_buffer.write_to_buffer((void *)vertices.data());

  // vertex buffer creation
  vertex_buffer_ = std::make_unique<buffer>(
    device_,
    vertex_size, // for calculating alignment
    vertex_count_, // same as above
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, // usage
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT// property
  );
  // copy the data from staging buffer to the vertex buffer
  device_.copy_buffer(staging_buffer.get_buffer(), vertex_buffer_->get_buffer(), buffer_size);
  // staging buffer is automatically freed in the dtor 
}

void mesh_model::create_index_buffers(const std::vector<uint32_t> &indices)
{
  index_count_ = static_cast<uint32_t>(indices.size());
  // if there is no index, nothing to do
  had_index_buffer_ = index_count_ > 0;
  if (!had_index_buffer_) return;

  VkDeviceSize buffer_size = sizeof(indices[0]) * index_count_;
  uint32_t indexSize = sizeof(indices[0]);

  // copy the data to the staging buffer
  buffer staging_buffer {
    device_,
    indexSize,
    index_count_,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  };

  staging_buffer.map();
  staging_buffer.write_to_buffer((void *)indices.data());

  index_buffer_ = std::make_unique<buffer> (
    device_,
    indexSize,
    index_count_,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT // optimal type of the memory type
  );

  // copy the data from staging buffer to the vertex buffer
  device_.copy_buffer(staging_buffer.get_buffer(), index_buffer_->get_buffer(), buffer_size);
}

void mesh_model::draw(VkCommandBuffer command_buffer)
{
  if (had_index_buffer_)
    vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
  else 
    vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
}

void mesh_model::bind(VkCommandBuffer command_buffer)
{
  VkBuffer buffers[] = {vertex_buffer_->get_buffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

  // last parameter should be same as the type of the Build::indices
  if (had_index_buffer_) 
    vkCmdBindIndexBuffer(command_buffer, index_buffer_->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
}

void mesh_builder::load_model(const std::string& filename)
{
  // loader
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
    throw std::runtime_error(warn + err);

  vertices.clear();
  indices.clear();

  std::unordered_map<vertex, uint32_t> unique_vertices{};


  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      vertex vertex{};
      // copy the vertex
      if (index.vertex_index >= 0) {
        vertex.position = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2]
        };
        // color support
        vertex.color = {
          attrib.colors[3 * index.vertex_index + 0],
          attrib.colors[3 * index.vertex_index + 1],
          attrib.colors[3 * index.vertex_index + 2]
        };
      }
      // copy the normal
      if (index.vertex_index >= 0) {
        vertex.normal = {
          attrib.normals[3 * index.normal_index + 0],
          attrib.normals[3 * index.normal_index + 1],
          attrib.normals[3 * index.normal_index + 2]
        };
      }
      // copy the texture coordinate
      if (index.vertex_index >= 0) {
        vertex.uv = {
          attrib.vertices[2 * index.texcoord_index + 0],
          attrib.vertices[2 * index.texcoord_index + 1]
        };
      }
      // if vertex is a new vertex
      if (unique_vertices.count(vertex) == 0) {
        unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(std::move(vertex));
      }
      indices.push_back(unique_vertices[vertex]);
    }
  }
}

std::vector<Eigen::Vector3d> mesh_model::get_vertex_position_list() const
{
  // extract position data from vertex_list_
  std::vector<Eigen::Vector3d> vertex_position_list;
  for (const auto &vertex : vertex_list_) {
    vertex_position_list.push_back(vertex.position.cast<double>());
  }
  return vertex_position_list;
}

graphics::vertex convert_geometry_to_graphics_vertex(const s_ptr<geometry::vertex>& gv)
{
  graphics::vertex res;
  res.position = gv->position_.cast<float>();
  res.color    = gv->color_.cast<float>();
  res.normal   = gv->normal_.cast<float>();
  res.uv       = gv->uv_.cast<float>();
  return res;
}

s_ptr<mesh_model> mesh_model::create_from_geometry_mesh_model(device& device, const s_ptr<geometry::mesh_model> &gm)
{
  graphics::mesh_builder builder;

  gm->align_vertex_id();
  std::vector<graphics::vertex> vertices(gm->get_vertex_count());
  std::vector<uint32_t> indices;
  // vertex_id is aligned to zero
  for (const auto& v : gm->get_vertex_map()) {
    vertices[v.first] = convert_geometry_to_graphics_vertex(v.second);
  }
  // search all faces
  // TODO : gpu cache friendly reconstruction
  for (const auto& kv : gm->get_face_map()) {
    auto he = kv.second->half_edge_;
    auto v0 = he->get_vertex();
    auto v1 = he->get_next()->get_vertex();
    auto v2 = he->get_next()->get_next()->get_vertex();
    indices.emplace_back(v0->id_);
    indices.emplace_back(v1->id_);
    indices.emplace_back(v2->id_);
  }

  builder.vertices = std::move(vertices);
  builder.indices  = std::move(indices);

  return std::make_shared<mesh_model>(device, builder);
}

} // namespace hnll::graphics