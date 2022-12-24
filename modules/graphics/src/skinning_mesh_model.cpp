// hnll
#include <graphics/skinning_mesh_model.hpp>
#include <graphics/device.hpp>
#include <graphics/buffer.hpp>

// std
#include <fstream>
#include <filesystem>
#include <iostream>

// lib
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>
#include <vulkan/vulkan.h>

namespace hnll::graphics {

u_ptr<descriptor_set_layout> skinning_mesh_model::desc_set_layout_ = nullptr;

void skinning_mesh_model::bind(VkCommandBuffer command_buffer, VkDescriptorSet global_desc_set, VkPipelineLayout pipeline_layout)
{
  // bind vertex buffer
  VkBuffer buffers[] = { vertex_buffer_->get_buffer() };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

  // bind index buffer
  vkCmdBindIndexBuffer(command_buffer, index_buffer_->get_buffer(), 0, VK_INDEX_TYPE_UINT32);

  // bind desc sets
  std::vector<VkDescriptorSet> desc_sets = { global_desc_set, desc_set_ };

  vkCmdBindDescriptorSets(
    command_buffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline_layout,
    0,
    static_cast<uint32_t>(desc_sets.size()),
    desc_sets.data(),
    0,
    nullptr
  );
}

void skinning_mesh_model::draw(VkCommandBuffer command_buffer)
{
  for (auto& node : root_nodes_) {
    draw_node(*node, command_buffer);
  }
  vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
}

u_ptr<skinning_mesh_model> skinning_mesh_model::create_from_gltf(const std::string &filepath, hnll::graphics::device &device)
{
  auto ret = std::make_unique<skinning_mesh_model>();

  ret->load_from_gltf(filepath, device);

  ret->setup_descs(device);

  return ret;
}

void skinning_mesh_model::setup_descs(device &device)
{
  create_desc_pool(device);
  create_desc_buffers(device);
  create_desc_sets();
}

void skinning_mesh_model::create_desc_pool(device &_device)
{
  desc_pool_ = descriptor_pool::builder(_device)
    .set_max_sets(1)
    .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
    .build();
}

void skinning_mesh_model::create_desc_buffers(device& _device)
{
  desc_buffer_ = graphics::buffer::create_with_staging(
    _device,
    sizeof(node_info_),
    1,
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    &node_info_
  );
}

void skinning_mesh_model::create_desc_sets()
{
  auto buffer_info = desc_buffer_->descriptor_info();
  auto desc_set_layout = get_desc_set_layout();
  descriptor_writer(*desc_set_layout_, *desc_pool_)
    .write_buffer(0, &buffer_info)
    .build(desc_set_);
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

  skinning_utils::skinning_model_builder builder;
  const auto& scene = gltf_model.scenes[0];
  // extract all root node indices
  for (const auto& node_index : scene.nodes) {
    root_nodes_.push_back(node_index);
  }

  load_node(gltf_model);
  load_mesh(gltf_model, builder);
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

  // create vertex buffer
  // convert visitor into vertex
  auto vertex_count = builder.position_buffer.size();
  std::vector<skinning_utils::vertex> vertices(vertex_count);
  for (int i = 0; i < vertex_count; i++) {
    vertices[i].position      = builder.position_buffer[i];
    vertices[i].normal        = builder.normal_buffer[i];
    vertices[i].tex_coord     = builder.tex_coord_buffer[i];
    vertices[i].joint_indices = builder.joint_buffer[i];
    vertices[i].joint_weights = builder.weight_buffer[i];
  }

  vertex_buffer_ = buffer::create_with_staging(
    device,
    sizeof(skinning_utils::vertex),
    vertex_count,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    vertices.data()
  );

  // create index buffer
  index_count_ = builder.index_buffer.size();
  auto size_index    = sizeof(uint32_t) * builder.index_buffer.size();
  index_buffer_ = buffer::create_with_staging(
    device,
    size_index,
    1,
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    builder.index_buffer.data()
  );

  skin_.skin_vertex_count = static_cast<uint32_t>(vertex_count);

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
  for (auto node_index : skin_.joints) {
    ret.emplace_back(nodes_[node_index]->name);
  }
  return ret;
}

std::vector<mat4> skinning_mesh_model::get_inv_bind_matrices() const
{ return skin_.inv_bind_matrices; }

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

void skinning_mesh_model::load_mesh(const tinygltf::Model &model, skinning_utils::skinning_model_builder &builder)
{
  auto& index_buffer     = builder.index_buffer;
  auto& position_buffer  = builder.position_buffer;
  auto& normal_buffer    = builder.normal_buffer;
  auto& tex_coord_buffer = builder.tex_coord_buffer;
  auto& joint_buffer     = builder.joint_buffer;
  auto& weight_buffer    = builder.weight_buffer;

  for (auto& in_mesh : model.meshes) {
    mesh_groups_.emplace_back(skinning_utils::mesh_group());
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
          tex_coord_buffer.emplace_back(vec2{0.f, 0.f});
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

      group.meshes.emplace_back(skinning_utils::mesh());
      auto& m = group.meshes.back();
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
    mesh_groups_[mesh_index].node_index = node_index;
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

  skin_.name = in_skin.name;
  skin_.joints.assign(in_skin.joints.begin(), in_skin.joints.end());

  if (in_skin.inverseBindMatrices > -1) {
    const auto& acc = model.accessors[in_skin.inverseBindMatrices];
    const auto& view = model.bufferViews[acc.bufferView];
    const auto& buffer = model.buffers[view.buffer];
    skin_.inv_bind_matrices.resize(acc.count);

    auto offset_bytes = acc.byteOffset + view.byteOffset;
    memcpy(
      skin_.inv_bind_matrices.data(),
      &buffer.data[offset_bytes],
      acc.count * sizeof(mat4)
    );
  }

  // copy to node_info
  node_info_.joint_count = skin_.joints.size();
  for (int i = 0; i < skin_.inv_bind_matrices.size(); i++) {
    node_info_.joint_matrices[i] = skin_.inv_bind_matrices[i];
  }
}

void skinning_mesh_model::load_material(const tinygltf::Model &model)
{
  for (const auto& in_material : model.materials) {
    materials_.emplace_back(skinning_utils::material());
    auto& target = materials_.back();
    target.name = in_material.name;

    for (auto& value : in_material.values) {
      auto value_name = value.first;
      if (value_name == "baseColorTexture") {
        auto texture_index = value.second.TextureIndex();
        target.texture_index = texture_index;
      }
//      if (value_name == "normalTexture") {
//        auto texture_index = value.second.TextureIndex();
//      }
      if (value_name == "baseColorFactor") {
        auto color = value.second.ColorFactor();
        target.diffuse_color = vec3(
          static_cast<float>(color[0]),
          static_cast<float>(color[1]),
          static_cast<float>(color[2])
        );
      }
    }
  }
}

void skinning_mesh_model::load_animation(const tinygltf::Model &model)
{
  for (auto& i_animation : model.animations) {
    skinning_utils::animation animation;
    animation.name = i_animation.name;
    if (i_animation.name.empty()) {
      animation.name = std::to_string(animations_.size());
    }

    // samplers
    for (auto& i_sampler : i_animation.samplers) {
      skinning_utils::animation_sampler sampler{};

      if (i_sampler.interpolation == "LINEAR") {
        sampler.interpolation = skinning_utils::interpolation_type::LINEAR;
      }
      if (i_sampler.interpolation == "STEP") {
        sampler.interpolation = skinning_utils::interpolation_type::STEP;
      }
      if (i_sampler.interpolation == "CUBICSPLINE") {
        sampler.interpolation = skinning_utils::interpolation_type::CUBICSPLINE;
      }

      // time values
      {
        const auto& accessor    = model.accessors[i_sampler.input];
        const auto& buffer_view = model.bufferViews[accessor.bufferView];
        const auto& buffer      = model.buffers[buffer_view.buffer];

        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

        const void*  data_ptr = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];
        const float* buf = static_cast<const float*>(data_ptr);

        for (size_t index = 0; index < accessor.count; index++) {
          sampler.inputs.push_back(buf[index]);
        }

        for (auto input : sampler.inputs) {
          if (input < animation.start) {
            animation.start = input;
          }
          if (input > animation.end) {
            animation.end = input;
          }
        }
      }

      // translation, rotation, scale
      {
        const auto& accessor    = model.accessors[i_sampler.output];
        const auto& buffer_view = model.bufferViews[accessor.bufferView];
        const auto& buffer      = model.buffers[buffer_view.buffer];

        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

        const void* data_ptr = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];

        switch (accessor.type) {
          case TINYGLTF_TYPE_VEC3 : {
            const vec3* buf = static_cast<const vec3*>(data_ptr);
            for (size_t index = 0; index < accessor.count; index++) {
              auto& v = buf[index];
              sampler.outputs.push_back({v.x(), v.y(), v.z(), 0.0f});
            }
            break;
          }
          case TINYGLTF_TYPE_VEC4 : {
            const vec4* buf = static_cast<const vec4*>(data_ptr);
            for (size_t index = 0; index < accessor.count; index++) {
              sampler.outputs.push_back(buf[index]);
            }
            break;
          }
          default : {
            std::cout << "unknown type" << std::endl;
            break;
          }
        }
      }
      animation.samplers.push_back(sampler);
    } // i_sampler loop

    // channel
    for (auto& i_channel : i_animation.channels) {
      skinning_utils::animation_channel channel{};

      if (i_channel.target_path == "translation") {
        channel.path = skinning_utils::animation_channel::path_type::TRANSLATION;
      }
      if (i_channel.target_path == "rotation") {
        channel.path = skinning_utils::animation_channel::path_type::ROTATION;
      }
      if (i_channel.target_path == "scale") {
        channel.path = skinning_utils::animation_channel::path_type::SCALE;
      }
      if (i_channel.target_path == "weights") {
        std::cout << "weights are not supported yet." << std::endl;
        continue;
      }
      channel.sampler_index = i_channel.sampler;
      channel.node = get_node(i_channel.target_node);
      if (!channel.node) {
        continue;
      }

      animation.channels.push_back(channel);
    }

    animations_.push_back(animation);
  } // animation loop
}

s_ptr<skinning_utils::node>& skinning_mesh_model::get_node(uint32_t index)
{
  s_ptr<skinning_utils::node> node_found = nullptr;
  for (auto& node : nodes_) {
    node_found = find_node(node, index);
    if (node_found) break;
  }
  return node_found;
}

s_ptr<skinning_utils::node>& skinning_mesh_model::find_node(s_ptr<skinning_utils::node>& parent, uint32_t index)
{
  s_ptr<skinning_utils::node> node_found = nullptr;
  if (parent->index == index) {
    return parent;
  }
  for (auto& child : parent->children) {
    node_found = find_node(child, index);
    if (node_found) break;
    return node_found;
  }
}

std::vector<VkVertexInputBindingDescription>   skinning_utils::vertex::get_binding_descriptions()
{
  std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
  binding_descriptions[0].binding = 0;
  binding_descriptions[0].stride = sizeof(skinning_utils::vertex);
  binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> skinning_utils::vertex::get_attribute_descriptions()
{
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
  attribute_descriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(skinning_utils::vertex, position)});
  attribute_descriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(skinning_utils::vertex, normal)});
  attribute_descriptions.push_back({2, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(skinning_utils::vertex, tex_coord)});
  attribute_descriptions.push_back({3, 0, VK_FORMAT_R32G32B32A32_UINT,   offsetof(skinning_utils::vertex, joint_indices)});
  attribute_descriptions.push_back({4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(skinning_utils::vertex, joint_weights)});

  return attribute_descriptions;
}
} // namespace hnll::graphics