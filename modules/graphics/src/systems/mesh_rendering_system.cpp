// hnll
#include <graphics/systems/mesh_rendering_system.hpp>
#include <game/components/mesh_component.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>
#include <string>

namespace hnll {

// should be compatible with a shader
struct MeshPushConstant
{
  glm::mat4 modelMatrix_m{1.0f};
  // to align data offsets with shader
  glm::mat4 normalMatrix_m{1.0f};
};

mesh_rendering_system::mesh_rendering_system
  (device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
  : rendering_system(device, render_type::SIMPLE)
{ 
  create_pipeline_layout(globalSetLayout);
  create_pipeline(renderPass);
}

mesh_rendering_system::~mesh_rendering_system()
{}

void mesh_rendering_system::create_pipeline_layout(VkDescriptorSetLayout globalSetLayout)
{
  // config push constant range
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  // mainly for if you are going to separate ranges for the vertex and fragment shaders
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(MeshPushConstant);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(device_.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS)
      throw std::runtime_error("failed to create pipeline layout!");
}

void mesh_rendering_system::create_pipeline(VkRenderPass renderPass)
{
  assert(pipeline_layout_ != nullptr && "cannot create pipeline before pipeline layout");

  pipeline_config_info pipelineConfig{};
  pipeline::default_pipeline_config_info(pipelineConfig);
  pipelineConfig.renderPass_m = renderPass;
  pipelineConfig.pipeline_layout_ = pipeline_layout_;
  pipeline_ = std::make_unique<pipeline>(
      device_,
      std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/simple_shader.vert.spv"), 
      std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/simple_shader.frag.spv"),
      pipelineConfig);
}


void mesh_rendering_system::render(frame_info frameInfo)
{
  pipeline_->bind(frameInfo.commandBuffer_m);

  vkCmdBindDescriptorSets(
    frameInfo.commandBuffer_m,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline_layout_,
    0, 1,
    &frameInfo.global_discriptor_set,
    0, nullptr
  );

  for (auto& target : render_target_map_) {
    
    auto obj = dynamic_cast<mesh_component*>(target.second.get());
    if (obj->get_model_sp() == nullptr) continue;
    MeshPushConstant push{};
    // camera projection
    push.modelMatrix_m = obj->get_transform().mat4();
    // automatically converse mat3(normalMatrix_m) to mat4 for shader data alignment
    push.normalMatrix_m = obj->get_transform().normal_matrix();

    vkCmdPushConstants(
        frameInfo.commandBuffer_m,
        pipeline_layout_, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        0, 
        sizeof(MeshPushConstant), 
        &push);
    obj->get_model_sp()->bind(frameInfo.commandBuffer_m);
    obj->get_model_sp()->draw(frameInfo.commandBuffer_m);
  }
}

} // namespace hve