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
  auto ret = std::make_unique<skinning_mesh_model>(device);

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

void get_node_props(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertex_count, size_t& index_count)
{
  if (node.children.size() > 0) {
    for (size_t i = 0; i < node.children.size(); i++) {
      get_node_props(model.nodes[node.children[i]], model, vertex_count, index_count);
    }
  }
  if (node.mesh > -1) {
    const tinygltf::Mesh mesh = model.meshes[node.mesh];
    for (size_t i = 0; i < mesh.primitives.size(); i++) {
      auto primitive = mesh.primitives[i];
      vertex_count += model.accessors[primitive.attributes.find("POSITION")->second].count;
      if (primitive.indices > -1) {
        index_count += model.accessors[primitive.indices].count;
      }
    }
  }
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
  if (!result) {
    std::cerr << "failed to load model : " << filepath << std::endl;
    return false;
  }

  skinning_utils::builder builder{};
  size_t vertex_count = 0;
  size_t index_count  = 0;

  const auto& scene = gltf_model.scenes[gltf_model.defaultScene > -1 ? gltf_model.defaultScene : 0];

  // count vertex and index
  for (size_t i = 0; i < scene.nodes.size(); i++) {
    get_node_props(gltf_model.nodes[scene.nodes[i]], gltf_model, vertex_count, index_count);
  }
  builder.vertex_buffer.resize(vertex_count);
  builder.index_buffer.resize(index_count);

  for (size_t i = 0; i < scene.nodes.size(); i++) {
    const auto& node = gltf_model.nodes[scene.nodes[i]];
    load_node(nullptr, node, scene.nodes[i], gltf_model, builder);
  }

  if (gltf_model.animations.size() > 0) {
    load_animation(gltf_model);
  }

  load_skins(gltf_model);

  for (auto node : linear_nodes_) {
    // assign skins
    if (node->skin_index > -1) {
      node->skin = skins_[node->skin_index];
    }
    // initial pose
    if (node->mesh_group) {
      node->update();
    }
  }

  // vertex and index buffers
  size_t vertex_buffer_size = vertex_count * sizeof(skinning_utils::vertex);
  size_t index_buffer_size  = index_count  * sizeof(uint32_t);
  assert(vertex_buffer_size > 0);

  vertex_buffer_ = buffer::create_with_staging(
    device_,
    vertex_buffer_size,
    1,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    builder.vertex_buffer.data()
  );
  index_buffer_ = buffer::create_with_staging(
    device_,
    index_buffer_size,
    1,
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    builder.index_buffer.data()
  );
}

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

void skinning_mesh_model::load_node(
  const s_ptr<skinning_utils::node>& parent,
  const tinygltf::Node&              node,
  uint32_t                           node_index,
  const tinygltf::Model&             model,
  skinning_utils::builder&           builder
)
{

}

void skinning_mesh_model::load_mesh(const tinygltf::Model &model, skinning_utils::skinning_model_builder &builder)
{

}

void skinning_mesh_model::load_skins(const tinygltf::Model &model)
{

}

void skinning_mesh_model::load_material(const tinygltf::Model &model)
{

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
} // namespace hnll::graphics