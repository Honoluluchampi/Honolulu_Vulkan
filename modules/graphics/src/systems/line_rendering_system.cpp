// hnll
#include <graphics/systems/line_rendering_system.hpp>

// hge
#include <game/components/line_component.hpp>

namespace hnll {

int LineRenderingSystem::interpolatingPointsCount = 4;

// should be compatible with a shader
// TODO use head and tail
struct LinePushConstant
{
  glm::vec4 position_{};
  glm::vec4 color_{1.f};
  float radius_;
};

LineRenderingSystem::LineRenderingSystem
  (device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
  : HveRenderingSystem(device, render_type::LINE)
{
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

LineRenderingSystem::~LineRenderingSystem()
{}

// TODO : take pushConstanctRange.size and virtualize this function
void LineRenderingSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(LinePushConstant);

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

void LineRenderingSystem::createPipeline(VkRenderPass renderPass)
{
  assert(pipelineLayout_m != nullptr && "cannot create pipeline before pipeline layout!");

  PipelineConfigInfo pipelineConfig{};
  HvePipeline::defaultPipelineConfigInfo(pipelineConfig);
  // for original configuration (dont use any vertex input attribute)
  pipelineConfig.attributeDescriptions.clear();
  pipelineConfig.bindingDescriptions.clear();

  pipelineConfig.renderPass_m = renderPass;
  pipelineConfig.pipelineLayout_m = pipelineLayout_m;
  hvePipeline_m = std::make_unique<HvePipeline>(
    hveDevice_m,
    std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/point_light.vert.spv"),
    std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/point_light.frag.spv"),
    pipelineConfig
  );
}

void LineRenderingSystem::render(FrameInfo frameInfo)
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
    
    auto obj = dynamic_cast<line_component*>(target.second.get());

    auto head2tail = obj->get_tail() - obj->get_head();
    // TODO : draw line
    for (int i = 1; i < interpolatingPointsCount + 1; i++) {
      // fill push constant
      LinePushConstant push{};
      push.position_ = glm::vec4(head2tail, 0.f);
      push.position_ *= (float)i / (interpolatingPointsCount + 1);
      push.position_ += glm::vec4(obj->get_head(), 0.f);
      push.color_ = glm::vec4(obj->get_color(), 0.f);
      push.radius_ = obj->get_radius();

      vkCmdPushConstants(
          frameInfo.commandBuffer_m,
          pipelineLayout_m, 
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
          0, 
          sizeof(LinePushConstant), 
          &push
      );
      vkCmdDraw(frameInfo.commandBuffer_m, 6, 1, 0, 0);
    }
  }
}
} // namespace hnll