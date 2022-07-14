// hnll
#include <graphics/systems/point_light.hpp>
#include <game/components/point_light_component.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
// lib
#include <glm/gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>

namespace hnll {

struct PointLightPushConstants
{
  glm::vec4 position_m{};
  glm::vec4 color_m{};
  float radius_m;
};

PointLightSystem::PointLightSystem
  (HveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
  : HveRenderingSystem(device, RenderType::POINT_LIGHT)
{
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem()
{ }

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
  // config push constant range
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  // mainly for if you are going to separate ranges for the vertex and fragment shaders
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PointLightPushConstants);

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

void PointLightSystem::createPipeline(VkRenderPass renderPass)
{
  assert(pipelineLayout_m != nullptr && "cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  HvePipeline::defaultPipelineConfigInfo(pipelineConfig);
  // for special config 
  pipelineConfig.attributeDescriptions.clear();
  pipelineConfig.bindingDescriptions.clear();

  pipelineConfig.renderPass_m = renderPass;
  pipelineConfig.pipelineLayout_m = pipelineLayout_m;
  hvePipeline_m = std::make_unique<HvePipeline>(
      hveDevice_m,
      std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/point_light.vert.spv"), 
      std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/point_light.frag.spv"),
      pipelineConfig);
}

void PointLightSystem::render(FrameInfo frameInfo)
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

  // copy the push constants
  for (auto& kv : renderTargetMap_m) {
    auto lightComp = dynamic_cast<PointLightComponent*>(kv.second.get());

    PointLightPushConstants push{};
    push.position_m = glm::vec4(lightComp->getTransform().translation_m, 1.f);
    push.color_m = glm::vec4(lightComp->getColor(), lightComp->getLightInfo().lightIntensity_m);
    push.radius_m = lightComp->getTransform().scale_m.x;

    vkCmdPushConstants(
      frameInfo.commandBuffer_m,
      pipelineLayout_m,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0, 
      sizeof(PointLightPushConstants),
      &push
    );
    vkCmdDraw(frameInfo.commandBuffer_m, 6, 1, 0, 0);
  }
}

} // namespace hve