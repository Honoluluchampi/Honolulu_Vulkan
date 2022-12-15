// hnll
#include <graphics/skinning_model.hpp>
#include <graphics/device.hpp>
#include <graphics/buffer.hpp>

// std
#include <fstream>
#include <filesystem>
#include <iostream>

// lib
#include <tiny_gltf/tiny_gltf.h>

namespace hnll::graphics {

u_ptr<skinning_model> skinning_model::create_from_gltf(const std::string &filepath, hnll::graphics::device &device)
{
  auto ret = std::make_unique<skinning_model>();

  ret->load_from_gltf(filepath, device);
}

bool load_file(std::vector<char>& out, const std::string& filepath)
{
  std::ifstream infile(filepath, std::ifstream::binary);
  if (!infile) { return false; }

  out.resize(infile.seekg(0, std::ifstream::end).tellg());
  infile.seekg(0, std::ifstream::beg).read(out.data(), out.size());

  return true;
}

bool skinning_model::load_from_gltf(const std::string &filepath, hnll::graphics::device &device)
{
  std::filesystem::path path = { filepath };
  std::vector<char> buffer;
  load_file(buffer, filepath);

  std::string base_dir = path.parent_path().string();

  std::string err, warn;
  tinygltf::TinyGLTF loader;
  tinygltf::Model gltf_model;
  bool result = false;

  if (path.extension() == L".glb") {
    result = loader.LoadBinaryFromMemory(
      &gltf_model,
      &err,
      &warn,
      reinterpret_cast<const uint8_t*>(buffer.data()),
      uint32_t(buffer.size()),
      base_dir
    );
  }

  if (!warn.empty()) { std::cerr << warn << std::endl; }
  if (!err.empty())  { std::cerr << err  << std::endl; }
  if (!result) { return false; }

  vertex_attribute_visitor visitor;
  const auto& scene = gltf_model.scenes[0];
  // extract all root node indices
  for (const auto& node_index : scene.nodes) {
    root_nodes_.push_back(node_index);
  }

  load_node(gltf_model);
  load_mesh(gltf_model, visitor);
  load_skin(gltf_model);
  load_material(gltf_model);

  auto memory_props  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  auto usage         = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  auto size_position = sizeof(vec3) * visitor.position_buffer.size();
  auto size_normal   = sizeof(vec3) * visitor.normal_buffer.size();
  auto size_texture  = sizeof(vec2) * visitor.tex_coord_buffer.size();
  auto size_index    = sizeof(uint32_t) * visitor.index_buffer.size();
  // create vertex attribute buffers
  vertex_attribute_buffer_.position_buffer = buffer::create(
    device,
    size_position,
    1,
    usage,
    memory_props,
    visitor.position_buffer.data()
  );

  vertex_attribute_buffer_.normal_buffer = buffer::create(
    device,
    size_normal,
    1,
    usage,
    memory_props,
    visitor.normal_buffer.data()
  );
  vertex_attribute_buffer_.tex_coord_buffer = buffer::create(
    device,
    size_texture,
    1,
    usage,
    memory_props,
    visitor.tex_coord_buffer.data()
  );

  // create index buffer
  index_buffer_ = buffer::create(
    device,
    size_index,
    1,
    usage,
    memory_props,
    visitor.index_buffer.data()
  );

  // extract skinning info
  if (has_skin_) {
    auto size_joint  = sizeof(uvec4) * visitor.joint_buffer.size();
    auto size_weight = sizeof(vec4) * visitor.weight_buffer.size();

    vertex_attribute_buffer_.joint_index_buffer = buffer::create(
      device,
      size_joint,
      1,
      usage,
      memory_props,
      visitor.joint_buffer.data()
    );
    vertex_attribute_buffer_.joint_weight_buffer = buffer::create(
      device,
      size_weight,
      1,
      usage,
      memory_props,
      visitor.weight_buffer.data()
    );

    skin_info_.skin_vertex_count = static_cast<uint32_t>(visitor.joint_buffer.size());
  }

  for (auto& image : gltf_model.images) {
    auto view = gltf_model.bufferViews[image.bufferView];
    auto offset_bytes = view.byteOffset;
    const void* src = &gltf_model.buffers[view.buffer].data[offset_bytes];

    images_.emplace_back();
    auto& info = images_.back();
    info.filepath = image.name;
    info.image_buffer.resize(view.byteLength);
    memcpy(info.image_buffer.data(), src, view.byteLength);
  }

  for (auto& texture : gltf_model.textures) {
    textures_.emplace_back();
    auto& info = textures_.back();
    info.image_index = texture.source;
  }

  return true;
}

std::vecto
} // namespace hnll::graphics