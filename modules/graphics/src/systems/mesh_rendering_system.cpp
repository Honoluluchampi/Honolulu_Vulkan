// hnll
#include <graphics/systems/mesh_rendering_system.hpp>
#include <game/components/mesh_component.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <eigen3/Eigen/Dense>

//std
#include <stdexcept>
#include <array>
#include <string>

using Eigen::Matrix4f;

namespace hnll {
namespace graphics {

// should be compatible with a shader
struct mesh_push_constant
{
  Matrix4f model_matrix = Matrix4f::Identity();
  // to align data offsets with shader
  Matrix4f normal_matrix = Matrix4f::Identity();
};

Matrix4f glm_to_eigen(const glm::mat4& glm_mat)
{
  Matrix4f eigen_mat;
  eigen_mat(0,0) = glm_mat[0][0];
  eigen_mat(0,1) = glm_mat[1][0];
  eigen_mat(0,2) = glm_mat[2][0];
  eigen_mat(0,3) = glm_mat[3][0];
  eigen_mat(1,0) = glm_mat[0][1];
  eigen_mat(1,1) = glm_mat[1][1];
  eigen_mat(1,2) = glm_mat[2][1];
  eigen_mat(1,3) = glm_mat[3][1];
  eigen_mat(2,0) = glm_mat[0][2];
  eigen_mat(2,1) = glm_mat[1][2];
  eigen_mat(2,2) = glm_mat[2][2];
  eigen_mat(2,3) = glm_mat[3][2];
  eigen_mat(3,0) = glm_mat[0][3];
  eigen_mat(3,1) = glm_mat[1][3];
  eigen_mat(3,2) = glm_mat[2][3];
  eigen_mat(3,3) = glm_mat[3][3];
  return eigen_mat;
}

mesh_rendering_system::mesh_rendering_system
  (device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout)
  : rendering_system(device, hnll::game::render_type::MESH)
{ 
  create_pipeline_layout(global_set_layout);
  create_pipeline(render_pass, "simple_shader.vert.spv", "simple_shader.frag.spv");
}

mesh_rendering_system::~mesh_rendering_system()
{}

void mesh_rendering_system::create_pipeline_layout(VkDescriptorSetLayout global_set_layout)
{
  // config push constant range
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  // mainly for if you are going to separate ranges for the vertex and fragment shaders
  push_constant_range.offset = 0;
  push_constant_range.size = sizeof(mesh_push_constant);

  std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
  pipeline_layout_info.pushConstantRangeCount = 1;
  pipeline_layout_info.pPushConstantRanges = &push_constant_range;
  if (vkCreatePipelineLayout(device_.get_device(), &pipeline_layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
      throw std::runtime_error("failed to create pipeline layout!");
}

void mesh_rendering_system::create_pipeline(
  VkRenderPass render_pass,
  std::string vertex_shader,
  std::string fragment_shader,
  std::string shaders_directory)
{
  assert(pipeline_layout_ != nullptr && "cannot create pipeline before pipeline layout");

  pipeline_config_info pipeline_config{};
  pipeline::default_pipeline_config_info(pipeline_config);
  pipeline_config.render_pass = render_pass;
  pipeline_config.pipeline_layout = pipeline_layout_;
  pipeline_ = std::make_unique<pipeline>(
      device_,
      shaders_directory + vertex_shader,
      shaders_directory + fragment_shader,
      pipeline_config);
}


void mesh_rendering_system::render(frame_info frame_info)
{
  pipeline_->bind(frame_info.command_buffer);

  vkCmdBindDescriptorSets(
    frame_info.command_buffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline_layout_,
    0, 1,
    &frame_info.global_discriptor_set,
    0, nullptr
  );

  for (auto& target : render_target_map_) {
    
    auto obj = dynamic_cast<hnll::game::mesh_component*>(target.second.get());
    if (obj->get_model_sp() == nullptr) continue;
    mesh_push_constant push{};
    // camera projection
    push.model_matrix = obj->get_transform().mat4().cast<float>();
    // automatically converse mat3(normal_matrix) to mat4 for shader data alignment
    push.normal_matrix = obj->get_transform().normal_matrix().cast<float>();

    vkCmdPushConstants(
        frame_info.command_buffer,
        pipeline_layout_, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        0, 
        sizeof(mesh_push_constant), 
        &push);
    obj->get_model_sp()->bind(frame_info.command_buffer);
    obj->get_model_sp()->draw(frame_info.command_buffer);
  }
}

} // namespace graphics
} // namespace hnll