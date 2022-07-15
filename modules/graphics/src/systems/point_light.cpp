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
  glm::vec4 color_{};
  float radius_m;
};

point_light_rendering_system::point_light_rendering_system
  (device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
  : rendering_system(device, render_type::POINT_LIGHT)
{
  create_pipeline_layout(globalSetLayout);
  create_pipeline(renderPass);
}

point_light_rendering_system::~point_light_rendering_system()
{ }

void point_light_rendering_system::create_pipeline_layout(VkDescriptorSetLayout globalSetLayout)
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
  if (vkCreatePipelineLayout(device_.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS)
      throw std::runtime_error("failed to create pipeline layout!");
}

void point_light_rendering_system::create_pipeline(VkRenderPass renderPass)
{
  assert(pipeline_layout_ != nullptr && "cannot create pipeline before pipeline layout");

  pipeline_config_info pipelineConfig{};
  pipeline::default_pipeline_config_info(pipelineConfig);
  // for special config 
  pipelineConfig.attribute_descriptions.clear();
  pipelineConfig.binding_descriptions.clear();

  pipelineConfig.renderPass_m = renderPass;
  pipelineConfig.pipeline_layout_ = pipeline_layout_;
  pipeline_ = std::make_unique<pipeline>(
      device_,
      std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/point_light.vert.spv"), 
      std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/point_light.frag.spv"),
      pipelineConfig);
}

void point_light_rendering_system::render(frame_info frameInfo)
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

  // copy the push constants
  for (auto& kv : render_target_map_) {
    auto lightComp = dynamic_cast<point_light_component*>(kv.second.get());

    PointLightPushConstants push{};
    push.position_m = glm::vec4(lightComp->get_transform().translation, 1.f);
    push.color_ = glm::vec4(lightComp->get_color(), lightComp->get_light_info().light_intensity);
    push.radius_m = lightComp->get_transform().scale.x;

    vkCmdPushConstants(
      frameInfo.commandBuffer_m,
      pipeline_layout_,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0, 
      sizeof(PointLightPushConstants),
      &push
    );
    vkCmdDraw(frameInfo.commandBuffer_m, 6, 1, 0, 0);
  }
}

} // namespace hve