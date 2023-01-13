// hnll
#include <graphics/meshlet_model.hpp>
#include <graphics/buffer.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <graphics/mesh_model.hpp>
#include <graphics/utils.hpp>
#include <geometry/mesh_separation.hpp>
#include <geometry/mesh_model.hpp>
#include <utils/utils.hpp>

// std
#include <iostream>

namespace hnll::graphics {

meshlet_model::meshlet_model(std::vector<vertex> &&raw_vertices, std::vector<meshlet> &&meshlets)
{
  raw_vertices_ = std::move(raw_vertices);
  meshlets_     = std::move(meshlets);
  meshlet_count_ = meshlets_.size();
}

u_ptr<meshlet_model> meshlet_model::create(
  device& _device,
  std::vector<vertex>&& _raw_vertices, std::vector<meshlet>&& _meshlets)
{
  auto ret = std::make_unique<meshlet_model>(
    std::move(_raw_vertices),
    std::move(_meshlets)
  );

  ret->setup_descs(_device);

  return ret;
}

u_ptr<meshlet_model> meshlet_model::create_from_file(hnll::graphics::device &_device, std::string _filename)
{
  std::vector<meshlet> meshlets;

  auto filepath = utils::get_full_path(_filename);
  // if model's cache exists
  if (geometry::mesh_separation::load_meshlet_cache(_filename, meshlets)) {
    mesh_builder builder;
    builder.load_model(filepath);
    return create(_device, std::move(builder.vertices), std::move(meshlets));
  }

  // prepare required data
  auto geometry_model = geometry::mesh_model::create_from_obj_file(filepath);
  meshlets = geometry::mesh_separation::separate(geometry_model, _filename);
  auto raw_vertices = geometry_model->move_raw_vertices();

  std::cout << "meshlet count : " << meshlets.size() << std::endl;
  return create(_device, std::move(raw_vertices), std::move(meshlets));
}

void meshlet_model::bind(
  VkCommandBuffer              _command_buffer,
  std::vector<VkDescriptorSet> _external_desc_set,
  VkPipelineLayout             _pipeline_layout)
{
  // prepare desc sets
  std::vector<VkDescriptorSet> desc_sets;
  for (const auto& set : _external_desc_set) {
    desc_sets.push_back(set);
  }
  for (const auto& set : desc_sets_) {
    desc_sets.push_back(set);
  }

  vkCmdBindDescriptorSets(
    _command_buffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    _pipeline_layout,
    0,
    static_cast<uint32_t>(desc_sets.size()),
    desc_sets.data(),
    0,
    nullptr
  );
}

void meshlet_model::draw(VkCommandBuffer _command_buffer)
{
  // draw
  vkCmdDrawMeshTasksNV(
    _command_buffer,
    meshlets_.size(),
    0
  );
}

void meshlet_model::setup_descs(device& _device)
{
  create_desc_pool(_device);
  create_desc_buffers(_device);
  create_desc_set_layouts(_device);
  create_desc_sets();
}

void meshlet_model::create_desc_pool(hnll::graphics::device &_device)
{
  desc_pool_ = descriptor_pool::builder(_device)
    .set_max_sets(DESC_SET_COUNT)
    .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DESC_SET_COUNT)
    .build();
}

void meshlet_model::create_desc_buffers(device& _device)
{
  desc_buffers_.resize(DESC_SET_COUNT);

  desc_buffers_[VERTEX_DESC_ID] = graphics::buffer::create_with_staging(
    _device,
    sizeof(vertex),
    raw_vertices_.size(),
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    raw_vertices_.data()
  );

  desc_buffers_[MESHLET_DESC_ID] = graphics::buffer::create_with_staging(
    _device,
    sizeof(meshlet),
    meshlets_.size(),
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    meshlets_.data()
  );
}

void meshlet_model::create_desc_set_layouts(device& _device)
{
  desc_set_layouts_.resize(DESC_SET_COUNT);

  for (int i = 0; i < DESC_SET_COUNT; i++) {
    desc_set_layouts_[i] = descriptor_set_layout::builder(_device)
      .add_binding(
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_SHADER_STAGE_TASK_BIT_NV | VK_SHADER_STAGE_MESH_BIT_NV)
      .build();
  }
}

void meshlet_model::create_desc_sets()
{
  desc_sets_.resize(DESC_SET_COUNT);

  for (int i = 0; i < DESC_SET_COUNT; i++) {
    auto buffer_info = desc_buffers_[i]->descriptor_info();
    descriptor_writer(*desc_set_layouts_[i], *desc_pool_)
      .write_buffer(0, &buffer_info)
      .build(desc_sets_[i]);
  }
}

// getter
const buffer& meshlet_model::get_vertex_buffer()  const
{ return *desc_buffers_[VERTEX_DESC_ID]; }

const buffer& meshlet_model::get_meshlet_buffer() const
{ return *desc_buffers_[MESHLET_DESC_ID]; }

std::vector<VkDescriptorSetLayout> meshlet_model::get_raw_desc_set_layouts() const
{
  std::vector<VkDescriptorSetLayout> ret;
  for (int i = 0; i < DESC_SET_COUNT; i++) {
    ret.push_back(desc_set_layouts_[i]->get_descriptor_set_layout());
  }
  return ret;
}

std::vector<u_ptr<descriptor_set_layout>> meshlet_model::default_desc_set_layouts(device& _device)
{
  std::vector<u_ptr<descriptor_set_layout>> ret;

  ret.resize(DESC_SET_COUNT);

  for (int i = 0; i < DESC_SET_COUNT; i++) {
    ret[i] = descriptor_set_layout::builder(_device)
      .add_binding(
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_SHADER_STAGE_TASK_BIT_NV | VK_SHADER_STAGE_MESH_BIT_NV)
      .build();
  }

  return ret;
}

} // namespace hnll::graphics