// hnll
#include <graphics/mesh_model.hpp>

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
struct hash<hnll::graphics::mesh_model::vertex>
{
  size_t operator() (hnll::graphics::mesh_model::vertex const &vertex) const
  {
    // stores final hash value
    size_t seed = 0;
    hnll::graphics::hash_combine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
    return seed;
  }
};

template <typename Scalar, int Rows, int Cols>
struct hash<Eigen::Matrix<Scalar, Rows, Cols>> {
  // https://wjngkoh.wordpress.com/2015/03/04/c-hash-function-for-eigen-matrix-and-vector/
  size_t operator()(const Eigen::Matrix<Scalar, Rows, Cols>& matrix) const {
    size_t seed = 0;
    for (size_t i = 0; i < static_cast<size_t>(matrix.size()); ++i) {
      Scalar elem = *(matrix.data() + i);
      seed ^=
          std::hash<Scalar>()(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};
} // namespace std

namespace hnll::graphics {

mesh_model::mesh_model(device& device, const mesh_model::builder &builder) : device_{device}
{
  create_vertex_buffers(builder.vertices);
  create_index_buffers(builder.indices);

  vertex_list_ = std::move(builder.vertices);
}

mesh_model::~mesh_model()
{
  // buffers will be freed in dtor of Hvebuffer
}

std::shared_ptr<mesh_model> mesh_model::create_model_from_file(device &device, const std::string &filename)
{
  builder builder;
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

std::vector<VkVertexInputBindingDescription> mesh_model::vertex::get_binding_descriptions()
{
  std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
  // per-vertex data is packed together in one array, so the index of the 
  // binding in the array is always 0
  binding_descriptions[0].binding   = 0;
  // number of bytes from one entry to the next
  binding_descriptions[0].stride    = sizeof(vertex);
  binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return binding_descriptions;
}
std::vector<VkVertexInputAttributeDescription> mesh_model::vertex::get_attribute_descriptions()
{
  // how to extract a vertex attribute from a chunk of vertex data
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

  // location, binding, format, offset
  attribute_descriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, position)});
  attribute_descriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, color)});
  attribute_descriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, normal)});
  attribute_descriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, uv)});

  return attribute_descriptions;
}

void mesh_model::builder::load_model(const std::string& filename)
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

} // namespace hnll::graphics