// hnll
#include <graphics/skinning_mesh_model.hpp>
#include <graphics/device.hpp>
#include <graphics/buffer.hpp>

// std
#include <fstream>
#include <filesystem>
#include <iostream>

// lib
#include <tiny_gltf/tiny_gltf.h>

namespace hnll::graphics {

u_ptr<skinning_mesh_model> skinning_mesh_model::create_from_gltf(const std::string &filepath, hnll::graphics::device &device)
{
  auto ret = std::make_unique<skinning_mesh_model>();

  ret->load_from_gltf(filepath, device);

  return ret;
}

bool load_file(std::vector<char>& out, const std::string& filepath)
{
  std::ifstream infile(filepath, std::ifstream::binary);
  if (!infile) { return false; }

  out.resize(infile.seekg(0, std::ifstream::end).tellg());
  infile.seekg(0, std::ifstream::beg).read(out.data(), out.size());

  return true;
}

bool skinning_mesh_model::load_from_gltf(const std::string &filepath, hnll::graphics::device &device)
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

  skinning_utils::vertex_attribute_visitor visitor;
  const auto& scene = gltf_model.scenes[0];
  // extract all root node indices
  for (const auto& node_index : scene.nodes) {
    root_nodes_.push_back(node_index);
  }

  load_node(gltf_model);
  load_mesh(gltf_model, visitor);
  load_skin(gltf_model);
  load_material(gltf_model);

  // for mesh shader
//  auto size_position = sizeof(vec3) * visitor.position_buffer.size();
//  auto size_normal   = sizeof(vec3) * visitor.normal_buffer.size();
//  auto size_texture  = sizeof(vec2) * visitor.tex_coord_buffer.size();
  // create vertex attribute buffers
//  vertex_attribute_buffer_.position_buffer = buffer::create(
//    device,
//    size_position,
//    1,
//    usage,
//    memory_props,
//    visitor.position_buffer.data()
//  );
//
//  vertex_attribute_buffer_.normal_buffer = buffer::create(
//    device,
//    size_normal,
//    1,
//    usage,
//    memory_props,
//    visitor.normal_buffer.data()
//  );
//  vertex_attribute_buffer_.tex_coord_buffer = buffer::create(
//    device,
//    size_texture,
//    1,
//    usage,
//    memory_props,
//    visitor.tex_coord_buffer.data()
//  );
//  // extract skinning info
//  if (has_skin_) {
//    auto size_joint  = sizeof(uvec4) * visitor.joint_buffer.size();
//    auto size_weight = sizeof(vec4) * visitor.weight_buffer.size();
//
//    vertex_attribute_buffer_.joint_index_buffer = buffer::create(
//      device,
//      size_joint,
//      1,
//      usage,
//      memory_props,
//      visitor.joint_buffer.data()
//    );
//    vertex_attribute_buffer_.joint_weight_buffer = buffer::create(
//      device,
//      size_weight,
//      1,
//      usage,
//      memory_props,
//      visitor.weight_buffer.data()
//    );
//  }

  // create index buffer
  auto size_index    = sizeof(uint32_t) * visitor.index_buffer.size();
  auto memory_props  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  auto usage         = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  index_buffer_ = buffer::create(
    device,
    size_index,
    1,
    usage,
    memory_props,
    visitor.index_buffer.data()
  );

    skin_info_.skin_vertex_count = static_cast<uint32_t>(visitor.joint_buffer.size());

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

std::vector<std::string> skinning_mesh_model::get_joint_node_names() const
{
  std::vector<std::string> ret;
  for (auto node_index : skin_info_.joints) {
    ret.emplace_back(nodes_[node_index]->get_name());
  }
  return ret;
}

std::vector<mat4> skinning_mesh_model::get_inv_bind_matrices() const
{ return skin_info_.inv_bind_matrices; }

vec3 vec3_convert_from_gltf(const double* input)
{
  return vec3 {
    static_cast<float>(input[0]),
    static_cast<float>(input[1]),
    static_cast<float>(input[2])
  };
}

quat quat_convert_from_gltf(const double* input)
{
  return quat {
    static_cast<float>(input[0]),
    static_cast<float>(input[1]),
    static_cast<float>(input[2]),
    static_cast<float>(input[3])
  };
}

void skinning_mesh_model::load_node(const tinygltf::Model &model)
{
  for (auto& input : model.nodes) {
    // create
    nodes_.emplace_back(std::make_shared<skinning_utils::node>());
    auto target = nodes_.back();

    // fill in values
    target->name = input.name;
    if (!input.translation.empty()) {
      target->translation = vec3_convert_from_gltf(input.translation.data());
    }
    if (!input.rotation.empty()) {
      target->rotation = quat_convert_from_gltf(input.rotation.data());
    }
    if (!input.scale.empty()) {
      target->scale = vec3_convert_from_gltf(input.scale.data());
    }
    for (auto& c : input.children) {
      target->children.push_back(c);
    }
    target->mesh_index = input.mesh;
  }
}

void skinning_mesh_model::load_mesh(const tinygltf::Model &model, hnll::graphics::skinning_mesh_model::vertex_attribute_visitor &visitor)
{
  auto& index_buffer     = visitor.index_buffer;
  auto& position_buffer  = visitor.position_buffer;
  auto& normal_buffer    = visitor.normal_buffer;
  auto& tex_coord_buffer = visitor.tex_coord_buffer;
  auto& joint_buffer     = visitor.joint_buffer;
  auto& weight_buffer    = visitor.weight_buffer;

  for (auto& in_mesh : model.meshes) {
    mesh_groups_.emplace_back(mesh_group());
    auto& group = mesh_groups_.back();

    for (auto& primitive : in_mesh.primitives) {
      auto index_start  = static_cast<uint32_t>(index_buffer.size());
      auto vertex_start = static_cast<uint32_t>(position_buffer.size());
      uint32_t index_count = 0, vertex_count = 0;
      bool has_skin = false;

      const auto& not_found = primitive.attributes.end();

      // extract positions
      if (auto attr = primitive.attributes.find("POSITION"); attr != not_found) {
        auto& acc  = model.accessors[attr->second];
        auto& view = model.bufferViews[acc.bufferView];
        auto offset_bytes = acc.byteOffset + view.byteOffset;
        const auto* src   = reinterpret_cast<const vec3*>(&(model.buffers[view.buffer].data[offset_bytes]));

        vertex_count = static_cast<uint32_t>(acc.count);
        for (uint32_t i = 0; i < vertex_count; i++) {
          position_buffer.push_back(src[i]);
        }
      }

      // extract normals
      if (auto attr = primitive.attributes.find("NORMAL"); attr != not_found) {
        auto& acc  = model.accessors[attr->second];
        auto& view = model.bufferViews[acc.bufferView];
        auto offset_bytes = acc.byteOffset + view.byteOffset;
        const auto* src   = reinterpret_cast<const vec3*>(&(model.buffers[view.buffer].data[offset_bytes]));

        vertex_count = static_cast<uint32_t>(acc.count);
        for (uint32_t i = 0; i < vertex_count; i++) {
          normal_buffer.push_back(src[i]);
        }
      }

      // extract texture coordinates
      if (auto attr = primitive.attributes.find("TEXCOORD_0"); attr != not_found) {
        auto& acc  = model.accessors[attr->second];
        auto& view = model.bufferViews[acc.bufferView];
        auto offset_bytes = acc.byteOffset + view.byteOffset;
        const auto* src   = reinterpret_cast<const vec2*>(&(model.buffers[view.buffer].data[offset_bytes]));

        for (uint32_t i = 0; i < vertex_count; i++) {
          tex_coord_buffer.push_back(src[i]);
        }
      } else {
        // zero pudding
        for (uint32_t i = 0; i < vertex_count; i++) {
          tex_coord_buffer.push_back(vec2{0.f, 0.f});
        }
      }

      // extract joint indices
      if (auto attr = primitive.attributes.find("JOINTS_0"); attr != not_found) {
        auto& acc  = model.accessors[attr->second];
        auto& view = model.bufferViews[acc.bufferView];
        auto offset_bytes = acc.byteOffset + view.byteOffset;
        const auto* src   = reinterpret_cast<const uint16_t*>(&(model.buffers[view.buffer].data[offset_bytes]));

        for (uint32_t i = 0; i < vertex_count; i++) {
          auto idx = i * 4;
          auto v = uvec4(
            src[idx + 0],
            src[idx + 1],
            src[idx + 2],
            src[idx + 3]
          );
          joint_buffer.push_back(v);
        }
      }

      // extract joint weights
      if (auto attr = primitive.attributes.find("WEIGHTS_0"); attr != not_found) {
        auto& acc  = model.accessors[attr->second];
        auto& view = model.bufferViews[acc.bufferView];
        auto offset_bytes = acc.byteOffset + view.byteOffset;
        const auto* src   = reinterpret_cast<const vec4*>(&(model.buffers[view.buffer].data[offset_bytes]));

        for (uint32_t i = 0; i < vertex_count; i++) {
          weight_buffer.push_back(src[i]);
        }
      }

      // extract index buffer
      {
        auto& acc = model.accessors[primitive.indices];
        const auto& view = model.bufferViews[acc.bufferView];
        const auto& buffer = model.buffers[view.buffer];
        index_count = static_cast<uint32_t>(acc.count);
        auto offset_bytes = acc.byteOffset + view.byteOffset;
        if (acc.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT) {
          auto src = reinterpret_cast<const uint32_t*>(&(buffer.data[offset_bytes]));

          for (size_t index = 0; index < acc.count; index++) {
            index_buffer.push_back(src[index]);
          }
        }
        if (acc.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT) {
          auto src = reinterpret_cast<const uint16_t*>(&(buffer.data[offset_bytes]));
          for (size_t index = 0; index < acc.count; index++) {
            index_buffer.push_back(src[index]);
          }
        }
      }

      group.meshes_.emplace_back(mesh());
      auto& m = group.meshes_.back();
      m.index_start    = index_start;
      m.vertex_start   = vertex_start;
      m.index_count    = index_count;
      m.vertex_count   = vertex_count;
      m.material_index = primitive.material;
    }
  }

  for (uint32_t node_index = 0; node_index < static_cast<uint32_t>(model.nodes.size()); node_index++) {
    auto mesh_index = model.nodes[node_index].mesh;
    if (mesh_index < 0)
      continue;
    mesh_groups_[mesh_index].node_index_ = node_index;
  }
}

void skinning_mesh_model::load_skin(const tinygltf::Model &model)
{
  if (model.skins.empty()) {
    has_skin_ = false;
    return;
  }
  has_skin_ = true;

  // only process first skin data
  const auto& in_skin = model.skins[0];

  skin_info_.name = in_skin.name;
  skin_info_.joints.assign(in_skin.joints.begin(), in_skin.joints.end());

  if (in_skin.inverseBindMatrices > -1) {
    const auto& acc = model.accessors[in_skin.inverseBindMatrices];
    const auto& view = model.bufferViews[acc.bufferView];
    const auto& buffer = model.buffers[view.buffer];
    skin_info_.inv_bind_matrices.resize(acc.count);

    auto offset_bytes = acc.byteOffset + view.byteOffset;
    memcpy(
      skin_info_.inv_bind_matrices.data(),
      &buffer.data[offset_bytes],
      acc.count * sizeof(mat4)
    );
  }
}

void skinning_mesh_model::load_material(const tinygltf::Model &model)
{
  for (const auto& in_material : model.materials) {
    materials_.emplace_back(material());
    auto& target = materials_.back();
    target.name_ = in_material.name;

    for (auto& value : in_material.values) {
      auto value_name = value.first;
      if (value_name == "baseColorTexture") {
        auto texture_index = value.second.TextureIndex();
        target.texture_index_ = texture_index;
      }
//      if (value_name == "normalTexture") {
//        auto texture_index = value.second.TextureIndex();
//      }
      if (value_name == "baseColorFactor") {
        auto color = value.second.ColorFactor();
        target.diffuse_color_ = vec3(
          static_cast<float>(color[0]),
          static_cast<float>(color[1]),
          static_cast<float>(color[2])
        );
      }
    }
  }
}
} // namespace hnll::graphics