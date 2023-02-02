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
#include <tiny_gltf/tiny_gltf.h>
#include <vulkan/vulkan.h>
#include <glm/gtc/type_ptr.hpp>

namespace hnll::graphics {

u_ptr<descriptor_set_layout> skinning_mesh_model::desc_set_layout_ = nullptr;

skinning_mesh_model::skinning_mesh_model(device &device) : device_(device){}

skinning_mesh_model::~skinning_mesh_model()
{
  for (auto& node : linear_nodes_) {
    node->mesh_group_.reset();
  }
}

void skinning_mesh_model::bind(VkCommandBuffer command_buffer, VkDescriptorSet global_desc_set, VkPipelineLayout pipeline_layout)
{
  // bind vertex buffer
  VkBuffer buffers[] = { vertex_buffer_->get_buffer() };
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

  // bind index buffer
  vkCmdBindIndexBuffer(command_buffer, index_buffer_->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
}

void skinning_mesh_model::draw(
  VkCommandBuffer command_buffer,
  VkDescriptorSet global_desc_set,
  VkPipelineLayout pipeline_layout,
  const skinning_mesh_push_constant& push)
{
  for (auto& node : nodes_) {
    draw_node(*node, command_buffer, global_desc_set, pipeline_layout, push);
  }
}

void skinning_mesh_model::draw_node(
  skinning_utils::node& node,
  VkCommandBuffer command_buffer,
  VkDescriptorSet global_desc_set,
  VkPipelineLayout pipeline_layout,
  const skinning_mesh_push_constant& push)
{
  if (node.mesh_group_) {
    for (auto& mesh : node.mesh_group_->meshes) {
      // bind desc sets
      const std::vector<VkDescriptorSet> descriptor_sets = {
        global_desc_set,
        node.mesh_group_->get_desc_set()
      };
      vkCmdBindDescriptorSets(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout,
        0,
        static_cast<uint32_t>(descriptor_sets.size()),
        descriptor_sets.data(),
        0,
        nullptr
      );

      // push constants
      vkCmdPushConstants(
        command_buffer,
        pipeline_layout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(skinning_mesh_push_constant),
        &push
      );

      vkCmdDrawIndexed(command_buffer, mesh.index_count, 1, mesh.first_index, 0, 0);
    }
  }
  for (auto& child : node.children) {
    draw_node(*child, command_buffer, global_desc_set, pipeline_layout, push);
  }
}

void skinning_mesh_model::update_animation(uint32_t index, float time)
{
  if (animations_.empty()) {
    std::cout << "gltf model does not contain animation." << std::endl;
    return;
  }
  if (index > static_cast<uint32_t>(animations_.size()) - 1) {
    std::cout << "No animation with index " << index << std::endl;
    return;
  }
  auto& animation = animations_[index];

  bool updated = false;
  for (auto& channel : animation.channels) {
    auto& sampler = animation.samplers[channel.sampler_index];
    if (sampler.inputs.size() > sampler.outputs.size()) {
      continue;
    }

    for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
      // choose current channel
      if ((time >= sampler.inputs[i]) && (time <= sampler.inputs[i+1])) {
        // normalize the elapsed time
        float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
        if (u <= 1.0f) {
          switch (channel.path) {
            case skinning_utils::animation_channel::path_type::TRANSLATION : {
              vec4 trans = (1.f - u) * sampler.outputs[i] + u * sampler.outputs[i + 1];
              channel.node_->translation = {trans.x(), trans.y(), trans.z() };
              break;
            }
            case skinning_utils::animation_channel::path_type::SCALE : {
              vec4 scale = (1.f - u) * sampler.outputs[i] + u * sampler.outputs[i + 1];
              channel.node_->scale = {scale.x(), scale.y(), scale.z() };
              break;
            }
            case skinning_utils::animation_channel::path_type::ROTATION : {
              quat q1;
              q1.x() = sampler.outputs[i].x();
              q1.y() = sampler.outputs[i].y();
              q1.z() = sampler.outputs[i].z();
              q1.w() = sampler.outputs[i].w();

              quat q2;
              q2.x() = sampler.outputs[i + 1].x();
              q2.y() = sampler.outputs[i + 1].y();
              q2.z() = sampler.outputs[i + 1].z();
              q2.w() = sampler.outputs[i + 1].w();

              channel.node_->rotation = q1.slerp(u, q2).normalized();
              break;
            }
          }
          updated = true;
        }
      }
    }
    if (updated)
      for (auto& node : nodes_)
        node->update();
  }
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
  descriptor_writer(*desc_set_layout_, *desc_pool_)
    .write_buffer(0, &buffer_info)
    .build(desc_set_);
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
  std::string err, warn;
  tinygltf::TinyGLTF gltf_context;
  tinygltf::Model    gltf_model;

  bool binary = false;

  size_t ext_pos = filepath.rfind('.', filepath.length());
  if (ext_pos != std::string::npos) {
    binary = (filepath.substr(ext_pos + 1, filepath.length() - ext_pos) == "glb");
  }

  bool file_loaded = binary ? gltf_context.LoadBinaryFromFile(&gltf_model, &err, &warn, filepath.c_str())
                            : gltf_context.LoadASCIIFromFile(&gltf_model, &err, &warn, filepath.c_str());

  size_t vertex_count = 0;
  size_t index_count  = 0;

  const auto& scene = gltf_model.scenes[gltf_model.defaultScene > -1 ? gltf_model.defaultScene : 0];

  // count vertex and index
  for (size_t i = 0; i < scene.nodes.size(); i++) {
    get_node_props(gltf_model.nodes[scene.nodes[i]], gltf_model, vertex_count, index_count);
  }
  builder_.vertex_buffer.resize(vertex_count);
  builder_.index_buffer.resize(index_count);

  for (size_t i = 0; i < scene.nodes.size(); i++) {
    const auto& node = gltf_model.nodes[scene.nodes[i]];
    load_node(nullptr, node, scene.nodes[i], gltf_model, builder_);
  }

  if (gltf_model.animations.size() > 0) {
    load_animation(gltf_model);
  }

  load_skins(gltf_model);

  for (auto node : linear_nodes_) {
    // assign skins
    if (node->skin_index > -1) {
      node->skin_ = skins_[node->skin_index];
    }
    // initial pose
    if (node->mesh_group_) {
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
    builder_.vertex_buffer.data()
  );
  index_buffer_ = buffer::create_with_staging(
    device_,
    index_buffer_size,
    1,
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    builder_.index_buffer.data()
  );

  return true;
}

template<int element_count, typename Input = float, typename Output = float>
Eigen::Matrix<Output, element_count, 1> vec_convert_from_raw(const Input* input)
{
  Eigen::Matrix<Output, element_count, 1> ret;
  for (int i = 0; i < element_count; i++) {
    ret[i] = static_cast<Output>(input[i]);
  };
  return ret;
}

mat4 mat4_convert_from_raw(const double* input)
{
  mat4 ret;
  ret << static_cast<float>(input[0]),
         static_cast<float>(input[1]),
         static_cast<float>(input[2]),
         static_cast<float>(input[3]),
         static_cast<float>(input[4]),
         static_cast<float>(input[5]),
         static_cast<float>(input[6]),
         static_cast<float>(input[7]),
         static_cast<float>(input[8]),
         static_cast<float>(input[9]),
         static_cast<float>(input[10]),
         static_cast<float>(input[11]),
         static_cast<float>(input[12]),
         static_cast<float>(input[13]),
         static_cast<float>(input[14]),
         static_cast<float>(input[15]);
  return ret;
}

void skinning_mesh_model::load_node(
  const s_ptr<skinning_utils::node>& parent,
  const tinygltf::Node&              node,
  uint32_t                           node_index,
  const tinygltf::Model&             model,
  skinning_utils::builder&           builder
)
{
  auto new_node = std::make_shared<skinning_utils::node>();
  new_node->index      = node_index;
  new_node->parent     = parent;
  new_node->name       = node.name;
  new_node->skin_index = node.skin;
  new_node->matrix     = mat4::Identity();

  // transform
  if (node.translation.size() == 3) {
    new_node->translation = vec_convert_from_raw<3, double>(node.translation.data());
  }
  if (node.rotation.size() == 4) {
    new_node->rotation = vec_convert_from_raw<4, double>(node.rotation.data());
  }
  if (node.scale.size() == 3) {
    new_node->scale = vec_convert_from_raw<3, double>(node.scale.data());
  }
  if (node.matrix.size() == 16) {
    new_node->matrix = mat4_convert_from_raw(node.matrix.data());
  }

  // load children
  if (node.children.size() > 0) {
    for (size_t i = 0; i < node.children.size(); i++) {
      load_node(new_node, model.nodes[node.children[i]], node.children[i], model, builder);
    }
  }

  // mesh
  if (node.mesh > -1) {
    const tinygltf::Mesh& mesh = model.meshes[node.mesh];
    auto new_mesh = std::make_shared<skinning_utils::mesh_group>(device_);
    for (size_t j = 0; j < mesh.primitives.size(); j++) {
      const tinygltf::Primitive& primitive = mesh.primitives[j];
      uint32_t vertex_start = static_cast<uint32_t>(builder.vertex_pos);
      uint32_t index_start  = static_cast<uint32_t>(builder.index_pos);
      uint32_t vertex_count = 0;
      uint32_t index_count  = 0;
      bool has_skin = false;
      bool has_indices = primitive.indices > -1;
      // vertices
      {
        const float* buffer_pos = nullptr;
        const float* buffer_normals = nullptr;
        const float* buffer_tex_coord_set_0 = nullptr;
        const float* buffer_tex_coord_set_1 = nullptr;
        const float* buffer_color_set_0 = nullptr;
        const void*  buffer_joints = nullptr;
        const float* buffer_weights = nullptr;

        int pos_byte_stride;
        int norm_byte_stride;
        int uv0_byte_stride;
        int uv1_byte_stride;
        int color_0_byte_stride;
        int joint_byte_stride;
        int weight_byte_stride;
        int joint_component_type;

        assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

        const auto& pos_acc  = model.accessors[primitive.attributes.find("POSITION")->second];
        const auto& pos_view = model.bufferViews[pos_acc.bufferView];
        buffer_pos = reinterpret_cast<const float*>(&model.buffers[pos_view.buffer].data[pos_acc.byteOffset + pos_view.byteOffset]);
        vertex_count = static_cast<uint32_t>(pos_acc.count);
        pos_byte_stride = pos_acc.ByteStride(pos_view) ? (pos_acc.ByteStride(pos_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

        if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
          const auto& norm_acc  = model.accessors[primitive.attributes.find("NORMAL")->second];
          const auto& norm_view = model.bufferViews[norm_acc.bufferView];
          buffer_normals = reinterpret_cast<const float*>(&model.buffers[norm_view.buffer].data[norm_acc.byteOffset + norm_view.byteOffset]);
          norm_byte_stride = norm_acc.ByteStride(norm_view) ? (norm_acc.ByteStride(norm_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
        }
        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
          const auto& uv0_acc  = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
          const auto& uv0_view = model.bufferViews[uv0_acc.bufferView];
          buffer_tex_coord_set_0 = reinterpret_cast<const float*>(&model.buffers[uv0_view.buffer].data[uv0_acc.byteOffset + uv0_view.byteOffset]);
          uv0_byte_stride = uv0_acc.ByteStride(uv0_view) ? (uv0_acc.ByteStride(uv0_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
        }
        if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
          const auto& uv1_acc  = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
          const auto& uv1_view = model.bufferViews[uv1_acc.bufferView];
          buffer_tex_coord_set_1 = reinterpret_cast<const float*>(&model.buffers[uv1_view.buffer].data[uv1_acc.byteOffset + uv1_view.byteOffset]);
          uv1_byte_stride = uv1_acc.ByteStride(uv1_view) ? (uv1_acc.ByteStride(uv1_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
        }
        if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) {
          const auto& col_acc  = model.accessors[primitive.attributes.find("COLOR_0")->second];
          const auto& col_view = model.bufferViews[col_acc.bufferView];
          buffer_color_set_0 = reinterpret_cast<const float*>(&model.buffers[col_view.buffer].data[col_acc.byteOffset + col_view.byteOffset]);
          color_0_byte_stride = col_acc.ByteStride(col_view) ? (col_acc.ByteStride(col_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
        }
        // skinning
        if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
          const auto& joint_acc  = model.accessors[primitive.attributes.find("JOINTS_0")->second];
          const auto& joint_view = model.bufferViews[joint_acc.bufferView];
          buffer_joints = reinterpret_cast<const float*>(&model.buffers[joint_view.buffer].data[joint_acc.byteOffset + joint_view.byteOffset]);
          joint_component_type = joint_acc.componentType;
          joint_byte_stride = joint_acc.ByteStride(joint_view) ? (joint_acc.ByteStride(joint_view) / tinygltf::GetComponentSizeInBytes(joint_component_type)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
        }
        if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
          const auto& weight_acc  = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
          const auto& weight_view = model.bufferViews[weight_acc.bufferView];
          buffer_weights = reinterpret_cast<const float*>(&model.buffers[weight_view.buffer].data[weight_acc.byteOffset + weight_view.byteOffset]);
          weight_byte_stride = weight_acc.ByteStride(weight_view) ? (weight_acc.ByteStride(weight_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
        }

        has_skin = buffer_joints && buffer_weights;

        for (size_t v = 0; v < pos_acc.count; v++) {
          auto& vert = builder.vertex_buffer[builder.vertex_pos];
          vert.position = vec_convert_from_raw<3>(&buffer_pos[v * pos_byte_stride]);
          vert.normal   = vec_convert_from_raw<3>(&buffer_normals[v * norm_byte_stride]).normalized();
          vert.tex_coord_0 = buffer_tex_coord_set_0 ? vec_convert_from_raw<2>(&buffer_tex_coord_set_0[v * uv0_byte_stride]) : vec2{ 0.f, 0.f };
          vert.tex_coord_1 = buffer_tex_coord_set_1 ? vec_convert_from_raw<2>(&buffer_tex_coord_set_1[v * uv1_byte_stride]) : vec2{ 0.f, 0.f };
          vert.color = buffer_color_set_0 ? vec_convert_from_raw<4>(&buffer_color_set_0[v * color_0_byte_stride]) : vec4{ 1.f, 1.f, 1.f, 1.f };

          if (has_skin) {
            switch (joint_component_type) {
              case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : {
                const auto *buf = static_cast<const uint16_t*>(buffer_joints);
                vert.joint_indices = vec_convert_from_raw<4, uint16_t, float>(&buf[v * joint_byte_stride]);
                break;
              }
              case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE : {
                const auto *buf = static_cast<const uint8_t*>(buffer_joints);
                vert.joint_indices = vec_convert_from_raw<4, uint8_t, float>(&buf[v * joint_byte_stride]);
                break;
              }
              default:
                std::cerr << "joint component type " << joint_component_type << " not supported." << std::endl;
                break;
            }
          }
          else {
            vert.joint_indices = vec4{ 0, 0, 0, 0 };
          }
          vert.joint_weights = has_skin ? vec_convert_from_raw<4>(&buffer_weights[v * weight_byte_stride]) : vec4{ 0.f, 0.f, 0.f, 0.f };
          if (vert.joint_weights.size() == 0.0f) {
            vert.joint_weights = vec4{ 1.f, 0.f, 0.f, 0.f };
          }
          builder.vertex_pos++;
        }
      }

      // indices
      if (has_indices) {
        const auto& acc  = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
        const auto& view = model.bufferViews[acc.bufferView];
        const auto& buffer = model.buffers[view.buffer];

        index_count = static_cast<uint32_t>(acc.count);
        const void* data = &buffer.data[acc.byteOffset + view.byteOffset];

        switch (acc.componentType) {
          case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT : {
            const uint32_t *buf = static_cast<const uint32_t*>(data);
            for (size_t index = 0; index < acc.count; index++) {
              builder.index_buffer[builder.index_pos] = buf[index] + vertex_start;
              builder.index_pos++;
            }
            break;
          }
          case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT : {
            const uint16_t *buf = static_cast<const uint16_t*>(data);
            for (size_t index = 0; index < acc.count; index++) {
              builder.index_buffer[builder.index_pos] = buf[index] + vertex_start;
              builder.index_pos++;
            }
            break;
          }
          case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE : {
            const uint8_t *buf = static_cast<const uint8_t*>(data);
            for (size_t index = 0; index < acc.count; index++) {
              builder.index_buffer[builder.index_pos] = buf[index] + vertex_start;
              builder.index_pos++;
            }
            break;
          }
          default :
            std::cerr << "index component type " << acc.componentType << " not supported." << std::endl;
            return;
        }
      }
      skinning_utils::mesh new_primitive;
      new_primitive.first_index = index_start;
      new_primitive.index_count = index_count;
      new_primitive.vertex_count = vertex_count;
      new_mesh->meshes.emplace_back(std::move(new_primitive));
    }
    new_node->mesh_group_ = new_mesh;
  }
  if (parent) {
    parent->children.push_back(new_node);
  } else {
    nodes_.push_back(new_node);
  }
  linear_nodes_.emplace_back(std::move(new_node));
}

void skinning_mesh_model::load_mesh(const tinygltf::Model &model, skinning_utils::skinning_model_builder &builder)
{

}

void skinning_mesh_model::load_skins(const tinygltf::Model &model)
{
  for (auto& skin : model.skins) {
    auto new_skin = std::make_shared<skinning_utils::skin>();
    new_skin->name = skin.name;

    // find root node
    if (skin.skeleton > -1) {
      new_skin->root_node = get_node(skin.skeleton);
    }

    // find joint nodes
    for (auto joint_index : skin.joints) {
      if (auto node = get_node(joint_index); node) {
        new_skin->joints.emplace_back(std::move(node));
      }
    }

    if (skin.inverseBindMatrices > -1) {
      const auto& acc = model.accessors[skin.inverseBindMatrices];
      const auto& buffer_view = model.bufferViews[acc.bufferView];
      const auto& buffer = model.buffers[buffer_view.buffer];
      new_skin->inv_bind_matrices.resize(acc.count);
      memcpy(new_skin->inv_bind_matrices.data(), &buffer.data[acc.byteOffset + buffer_view.byteOffset], acc.count * sizeof(mat4));
    }

    skins_.emplace_back(std::move(new_skin));
  }
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
      channel.node_ = get_node(i_channel.target_node);
      if (!channel.node_) {
        continue;
      }

      animation.channels.push_back(channel);
    }

    animations_.push_back(animation);
  } // animation loop
}

s_ptr<skinning_utils::node> skinning_mesh_model::get_node(uint32_t index)
{
  s_ptr<skinning_utils::node> node_found = nullptr;
  for (auto& node : nodes_) {
    node_found = find_node(node, index);
    if (node_found) break;
  }
  return node_found;
}

s_ptr<skinning_utils::node> skinning_mesh_model::find_node(s_ptr<skinning_utils::node>& parent, uint32_t index)
{
  s_ptr<skinning_utils::node> node_found = nullptr;
  if (parent->index == index) {
    return parent;
  }
  for (auto& child : parent->children) {
    node_found = find_node(child, index);
    if (node_found) break;
  }
  return node_found;
}
} // namespace hnll::graphics