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

namespace hve {

// should be compatible with a shader
struct SimplePushConstantData
{
  glm::mat4 transform_m{1.0f};
  // to align data offsets with shader
  glm::mat4 normalMatrix_m{1.0f};
};

SimpleRendererSystem::SimpleRendererSystem(HveDevice& device, VkRenderPass renderPass) : hveDevice_m(device)
{
  createPipelineLayout();
  createPipeline(renderPass);
}

SimpleRendererSystem::~SimpleRendererSystem()
{
  vkDestroyPipelineLayout(hveDevice_m.device(), pipelineLayout_m, nullptr);
}

void SimpleRendererSystem::createPipelineLayout()
{
  // config push constant range
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  // mainly for if you are going to separate ranges for the vertex and fragment shaders
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
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
      "./shader/spv/vert.spv", 
      "./shader/spv/frag.spv",
      pipelineConfig);
}


void SimpleRendererSystem::renderGameObjects(FrameInfo frameInfo, std::vector<HveGameObject>& gameObjects)
{
  hvePipeline_m->bind(frameInfo.CommandBuffer_m);

  auto projectionView = frameInfo.camera_m.getProjection() * frameInfo.camera_m.getView();

  for (auto& obj : gameObjects) {
    // rotate around the y axis
    // obj.transform_m.rotation_m.y = glm::mod(obj.transform_m.rotation_m.y + 0.01f, glm::two_pi<float>());
    // obj.transform_m.rotation_m.x = glm::mod(obj.transform_m.rotation_m.x + 0.005f, glm::two_pi<float>());

    SimplePushConstantData push{};
    auto modelMatrix = obj.transform_m.mat4();
    // camera projection
    push.transform_m = projectionView * modelMatrix;
    // automatically converse mat3(normalMatrix_m) to mat4 for shader data alignment
    push.normalMatrix_m = obj.transform_m.normalMatrix();

    vkCmdPushConstants(
        frameInfo.CommandBuffer_m,
        pipelineLayout_m, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        0, 
        sizeof(SimplePushConstantData), 
        &push);
    obj.model_m->bind(frameInfo.CommandBuffer_m);
    obj.model_m->draw(frameInfo.CommandBuffer_m);
  }
}

} // namespace hve