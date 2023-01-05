// hnll
#include <graphics/frame_anim_mesh_model.hpp>
#include <graphics/skinning_mesh_model.hpp>

namespace hnll::graphics {

#define MAX_FPS 60

frame_anim_mesh_model::frame_anim_mesh_model(device &_device) : device_(_device) {}

u_ptr<frame_anim_mesh_model> frame_anim_mesh_model::create_from_skinning_mesh_model(device& _device, skinning_mesh_model &original, uint32_t max_fps = MAX_FPS)
{
  auto ret = std::make_unique<frame_anim_mesh_model>(_device);
  ret->load_from_skinning_mesh_model(original, max_fps);

  return ret;
}

void frame_anim_mesh_model::load_from_skinning_mesh_model(hnll::graphics::skinning_mesh_model &original, uint32_t max_fps)
{
  auto original_data = original.get_ownership_of_builder();

  // extract common attributes
  std::vector<common_attributes> common_attribs;
  for (auto& data : original_data.vertex_buffer) {
    common_attributes new_ca;
    new_ca.uv0     = data.tex_coord_0;
    new_ca.uv1     = data.tex_coord_1;
    new_ca.color   = data.color;
    new_ca.joints  = data.joint_indices;
    new_ca.weights = data.joint_weights;
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
    original_data.index_buffer.size() * sizeof(uint32_t),
    1,
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    original_data.index_buffer.data()
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
      auto new_dynamic_attribs = extract_dynamic_attributes(original, original_data);

      // assign buffer
      auto new_buffer = buffer::create_with_staging(
        device_,
        new_dynamic_attribs.size() * sizeof(dynamic_attributes),
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
}

void extract_node_vertices(
  skinning_utils::node& node,
  std::vector<frame_anim_mesh_model::dynamic_attributes>& buffer,
  skinning_utils::builder& builder,
  std::vector<bool>& vertex_computed)
{
  for (auto& mesh : node.mesh_group_->meshes) {
    for (int i = 0; i < mesh.index_count; i++) {
      uint32_t index = builder.index_buffer[i + mesh.first_index];
      // already calculated
      if (vertex_computed[index])
        continue;

    }
  }
}

std::vector<frame_anim_mesh_model::dynamic_attributes>
  frame_anim_mesh_model::extract_dynamic_attributes(
    skinning_mesh_model& original,
    skinning_utils::builder& builder)
{
  std::vector<dynamic_attributes> new_dynamic_attribs(vertex_count_);
  std::vector<bool> vertex_computed(vertex_count_, false);

  for (auto& node : original.get_nodes()) {
    if (node->mesh_group_) {
      extract_node_vertices(*node, new_dynamic_attribs, builder, vertex_computed);
    }
    for (auto& child : node->children) {
      extract_node_vertices(*child, new_dynamic_attribs, builder, vertex_computed);
    }
  }

  return new_dynamic_attribs;
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
  attribute_descriptions.push_back({5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(common_attributes, joints)});
  attribute_descriptions.push_back({6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(common_attributes, weights)});

  return attribute_descriptions;
}

} // namespace hnll::graphics