// hnll
#include <graphics/buffer.hpp>
#include <graphics/frame_anim_mesh_model.hpp>
#include <graphics/skinning_mesh_model.hpp>

namespace hnll::graphics {

#define MAX_FPS 60

frame_anim_mesh_model::frame_anim_mesh_model(device &_device) : device_(_device) {}

u_ptr<frame_anim_mesh_model> frame_anim_mesh_model::create_from_skinning_mesh_model(device& _device, skinning_mesh_model &original, uint32_t max_fps)
{
  auto ret = std::make_unique<frame_anim_mesh_model>(_device);
  ret->load_from_skinning_mesh_model(original, max_fps);

  return ret;
}

void frame_anim_mesh_model::load_from_skinning_mesh_model(hnll::graphics::skinning_mesh_model &original, uint32_t max_fps)
{
  auto builder = original.get_ownership_of_builder();

  vertex_count_ = builder.vertex_buffer.size();
  index_count_ = builder.index_buffer.size();

  // extract time info
  auto& animations = original.get_animations();
  start_times_.resize(animations.size());
  end_times_.resize(animations.size());
  for (int i = 0; i < animations.size(); i++) {
    start_times_[i] = animations[i].start;
    end_times_[i]   = animations[i].end;
  }

  // extract common attributes
  std::vector<common_attributes> common_attribs;
  for (auto& data : builder.vertex_buffer) {
    common_attributes new_ca;
    new_ca.uv0     = data.tex_coord_0;
    new_ca.uv1     = data.tex_coord_1;
    new_ca.color   = data.color;
    common_attribs.emplace_back(std::move(new_ca));
  }
  common_attributes_buffer_ = buffer::create_with_staging(
    device_,
    common_attribs.size() * sizeof(common_attributes),
    1,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    common_attribs.data()
  );

  // extract index buffer
  index_buffer_ = buffer::create_with_staging(
    device_,
    builder.index_buffer.size() * sizeof(uint32_t),
    1,
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    builder.index_buffer.data()
  );

  // extract dynamic attributes
  auto animation_count = original.get_animations().size();
  dynamic_attributes_buffers_.resize(animation_count);
  for (int i = 0; i < animation_count; i++) {
    auto& anim = original.get_animations()[i];
    float timer = anim.start;
    while (timer <= anim.end) {
      // calculate new position and normal
      original.update_animation(i, timer);
      auto new_dynamic_attribs = extract_dynamic_attributes(original, builder);

      // assign buffer
      auto new_buffer = buffer::create_with_staging(
        device_,
        144 * sizeof(dynamic_attributes),
        1,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        new_dynamic_attribs.data()
      );
      dynamic_attributes_buffers_[i].emplace_back(std::move(new_buffer));

      // update timer
      timer += 1.f / static_cast<float>(max_fps);
    }
  }

  frame_counts_.resize(dynamic_attributes_buffers_.size());
  for (int i = 0; i < dynamic_attributes_buffers_.size(); i++) {
    frame_counts_[i] = dynamic_attributes_buffers_[i].size();
  }
}

Eigen::Matrix3f mat4to3(const mat4& original)
{
  Eigen::Matrix3f ret;
  ret << original(0, 0), original(0, 1), original(0, 2),
         original(1, 0), original(1, 1), original(1, 2),
         original(2, 0), original(2, 1), original(2, 2);
  return ret;
}

void extract_node_vertices(
  skinning_utils::node& node,
  std::vector<frame_anim_mesh_model::dynamic_attributes>& buffer,
  const skinning_utils::builder& builder,
  std::vector<bool>& vertex_computed)
{
  if (node.mesh_group_ == nullptr)
    return;

  for (auto& mesh : node.mesh_group_->meshes) {
    for (int i = 0; i < mesh.index_count; i++) {
      uint32_t index = builder.index_buffer[i + mesh.first_index];
      // already calculated
      if (vertex_computed[index])
        continue;

      // compute vertex position and normal
      const auto& target = builder.vertex_buffer[index];
      vec3 new_position, new_normal;
      if (node.mesh_group_->block.joint_count > 0) {
        const auto &joint_matrices = node.mesh_group_->block.joint_matrices;
        mat4 skin_mat = target.joint_weights.x() * joint_matrices[static_cast<int>(target.joint_indices.x())]
                      + target.joint_weights.y() * joint_matrices[static_cast<int>(target.joint_indices.y())]
                      + target.joint_weights.z() * joint_matrices[static_cast<int>(target.joint_indices.z())]
                      + target.joint_weights.w() * joint_matrices[static_cast<int>(target.joint_indices.w())];
        mat4 transform_mat = node.mesh_group_->block.matrix * skin_mat;
        vec4 position = transform_mat * vec4{ target.position.x(), target.position.y(), target.position.z(), 1.f};
        new_position = vec3{ position.x(), position.y(), position.z() };
        new_normal = mat4to3(transform_mat) * target.normal;
      }
      else {
        vec4 position = node.mesh_group_->block.matrix * vec4{ target.position.x(), target.position.y(), target.position.z(), 1.f};
        new_position = vec3{ position.x(), position.y(), position.z() };
        new_normal = mat4to3(node.mesh_group_->block.matrix) * target.normal;
      }
      buffer[index] = { new_position, new_normal };
      vertex_computed[index] = true;
    }
  }
}

std::vector<frame_anim_mesh_model::dynamic_attributes>
  frame_anim_mesh_model::extract_dynamic_attributes(
    skinning_mesh_model& original,
    const skinning_utils::builder& builder)
{
  std::vector<dynamic_attributes> new_dynamic_attribs(vertex_count_);
  std::vector<bool> vertex_computed(vertex_count_, false);

  for (auto& node : original.get_nodes()) {
    extract_node_vertices(*node, new_dynamic_attribs, builder, vertex_computed);
    for (auto& child : node->children) {
      extract_node_vertices(*child, new_dynamic_attribs, builder, vertex_computed);
    }
  }

  return new_dynamic_attribs;
}

void frame_anim_mesh_model::bind(uint32_t animation_index, uint32_t frame_index, VkCommandBuffer command_buffer)
{
  // bind vertex buffer
  VkBuffer vertex_buffers[] = {
    dynamic_attributes_buffers_[animation_index][frame_index]->get_buffer(),
    common_attributes_buffer_->get_buffer()
  };
  VkDeviceSize offsets[] = { 0, 0 };
  vkCmdBindVertexBuffers(command_buffer, 0, 2, vertex_buffers, offsets);

  vkCmdBindIndexBuffer(command_buffer, index_buffer_->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
}

void frame_anim_mesh_model::draw(VkCommandBuffer command_buffer)
{
  vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
}

std::vector<VkVertexInputBindingDescription>   frame_anim_mesh_model::get_binding_descriptions()
{
  std::vector<VkVertexInputBindingDescription> binding_descriptions;
  binding_descriptions.resize(2);
  // binding for dynamic attribs
  binding_descriptions[0].binding   = 0;
  binding_descriptions[0].stride    = sizeof(dynamic_attributes);
  binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  // binding for common attribs
  binding_descriptions[1].binding   = 1;
  binding_descriptions[1].stride    = sizeof(common_attributes);
  binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> frame_anim_mesh_model::get_attributes_descriptions()
{
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
  attribute_descriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(dynamic_attributes, position)});
  attribute_descriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT,    offsetof(dynamic_attributes, normal)});
  attribute_descriptions.push_back({2, 1, VK_FORMAT_R32G32_SFLOAT,       offsetof(common_attributes, uv0)});
  attribute_descriptions.push_back({3, 1, VK_FORMAT_R32G32_SFLOAT,       offsetof(common_attributes, uv1)});
  attribute_descriptions.push_back({4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(common_attributes, color)});

  return attribute_descriptions;
}

} // namespace hnll::graphics