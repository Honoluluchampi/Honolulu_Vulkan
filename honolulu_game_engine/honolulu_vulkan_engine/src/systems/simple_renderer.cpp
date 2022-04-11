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

  glm::mat4 Transform::mat4() 
{
  const float c3 = glm::cos(rotation_m.z), s3 = glm::sin(rotation_m.z), c2 = glm::cos(rotation_m.x), 
    s2 = glm::sin(rotation_m.x), c1 = glm::cos(rotation_m.y), s1 = glm::sin(rotation_m.y);
  return glm::mat4{
      {
          scale_m.x * (c1 * c3 + s1 * s2 * s3),
          scale_m.x * (c2 * s3),
          scale_m.x * (c1 * s2 * s3 - c3 * s1),
          0.0f,
      },
      {
          scale_m.y * (c3 * s1 * s2 - c1 * s3),
          scale_m.y * (c2 * c3),
          scale_m.y * (c1 * c3 * s2 + s1 * s3),
          0.0f,
      },
      {
          scale_m.z * (c2 * s1),
          scale_m.z * (-s2),
          scale_m.z * (c1 * c2),
          0.0f,
      },
      {translation_m.x, translation_m.y, translation_m.z, 1.0f}};
}

// normal = R * S(-1)
glm::mat3 Transform::normalMatrix()
{
  const float c3 = glm::cos(rotation_m.z), s3 = glm::sin(rotation_m.z), c2 = glm::cos(rotation_m.x), 
    s2 = glm::sin(rotation_m.x), c1 = glm::cos(rotation_m.y), s1 = glm::sin(rotation_m.y);

  const glm::vec3 invScale = 1.0f / scale_m;
  return glm::mat3{
      {
          invScale.x * (c1 * c3 + s1 * s2 * s3),
          invScale.x * (c2 * s3),
          invScale.x * (c1 * s2 * s3 - c3 * s1)
      },
      {
          invScale.y * (c3 * s1 * s2 - c1 * s3),
          invScale.y * (c2 * c3),
          invScale.y * (c1 * c3 * s2 + s1 * s3)
      },
      {
          invScale.z * (c2 * s1),
          invScale.z * (-s2),
          invScale.z * (c1 * c2)
      }
  };
}

// should be compatible with a shader
struct SimplePushConstantData
{
  glm::mat4 modelMatrix_m{1.0f};
  // to align data offsets with shader
  glm::mat4 normalMatrix_m{1.0f};
};

SimpleRendererSystem::SimpleRendererSystem
  (HveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
  : HveRenderingSystem(device, RenderType::SIMPLE)
{ 
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

SimpleRendererSystem::~SimpleRendererSystem()
{}

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

  for (auto& target : renderTargetMap_m) {
    auto& obj = *target.second;
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