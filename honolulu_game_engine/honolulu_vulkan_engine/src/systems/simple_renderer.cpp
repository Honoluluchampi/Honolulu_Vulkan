#include <simple_renderer.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
// lib
#include <glm/gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>
#include <string>

namespace hnll {

// should be compatible with a shader
struct SimplePushConstantData
{
  glm::mat4 modelMatrix_m{1.0f};
  // to align data offsets with shader
  glm::mat4 normalMatrix_m{1.0f};
};

SimpleRendererSystem::SimpleRendererSystem(HveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : hveDevice_m(device)
{
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

SimpleRendererSystem::~SimpleRendererSystem()
{
  vkDestroyPipelineLayout(hveDevice_m.device(), pipelineLayout_m, nullptr);
}

void SimpleRendererSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
  // config push constant range
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  // mainly for if you are going to separate ranges for the vertex and fragment shaders
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(hveDevice_m.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout_m) != VK_SUCCESS)
      throw std::runtime_error("failed to create pipeline layout!");
}

void SimpleRendererSystem::createPipeline(VkRenderPass renderPass)
{
  assert(pipelineLayout_m != nullptr && "cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  HvePipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass_m = renderPass;
  pipelineConfig.pipelineLayout_m = pipelineLayout_m;
  hvePipeline_m = std::make_unique<HvePipeline>(
      hveDevice_m,
      std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/simple_shader.vert.spv"), 
      std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/simple_shader.frag.spv"),
      pipelineConfig);
}


void SimpleRendererSystem::render(FrameInfo frameInfo)
{
  hvePipeline_m->bind(frameInfo.commandBuffer_m);

  vkCmdBindDescriptorSets(
    frameInfo.commandBuffer_m,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout_m,
    0, 1,
    &frameInfo.globalDiscriptorSet_m,
    0, nullptr
  );

  for (auto& modelCmpt : frameInfo.modelMap_m) {
    auto& obj = *modelCmpt.second;
    if (obj.getSpModel() == nullptr) continue;
    SimplePushConstantData push{};
    // camera projection
    push.modelMatrix_m = obj.getTransform().mat4();
    // automatically converse mat3(normalMatrix_m) to mat4 for shader data alignment
    push.normalMatrix_m = obj.getTransform().normalMatrix();

    vkCmdPushConstants(
        frameInfo.commandBuffer_m,
        pipelineLayout_m, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        0, 
        sizeof(SimplePushConstantData), 
        &push);
    obj.getSpModel()->bind(frameInfo.commandBuffer_m);
    obj.getSpModel()->draw(frameInfo.commandBuffer_m);
  }
}

} // namespace hve