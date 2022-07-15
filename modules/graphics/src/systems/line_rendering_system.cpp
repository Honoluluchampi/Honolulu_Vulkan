// hnll
#include <graphics/systems/line_rendering_system.hpp>

// hge
#include <game/components/line_component.hpp>

namespace hnll {

int line_rendering_system::inter_polating_points_count = 4;

// should be compatible with a shader
// TODO use head and tail
struct LinePushConstant
{
  glm::vec4 position_{};
  glm::vec4 color_{1.f};
  float radius_;
};

line_rendering_system::line_rendering_system
  (device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
  : rendering_system(device, render_type::LINE)
{
  create_pipeline_layout(globalSetLayout);
  create_pipeline(renderPass);
}

line_rendering_system::~line_rendering_system()
{}

// TODO : take pushConstanctRange.size and virtualize this function
void line_rendering_system::create_pipeline_layout(VkDescriptorSetLayout globalSetLayout)
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
  if (vkCreatePipelineLayout(device_.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS)
    throw std::runtime_error("failed to create pipeline layout!");
}

void line_rendering_system::create_pipeline(VkRenderPass renderPass)
{
  assert(pipeline_layout_ != nullptr && "cannot create pipeline before pipeline layout!");

  pipeline_config_info pipelineConfig{};
  pipeline::default_pipeline_config_info(pipelineConfig);
  // for original configuration (dont use any vertex input attribute)
  pipelineConfig.attribute_descriptions.clear();
  pipelineConfig.binding_descriptions.clear();

  pipelineConfig.renderPass_m = renderPass;
  pipelineConfig.pipeline_layout_ = pipeline_layout_;
  pipeline_ = std::make_unique<pipeline>(
    device_,
    std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/point_light.vert.spv"),
    std::string(std::getenv("HVE_DIR")) + std::string("/shader/spv/point_light.frag.spv"),
    pipelineConfig
  );
}

void line_rendering_system::render(frame_info frameInfo)
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
    
    auto obj = dynamic_cast<line_component*>(target.second.get());

    auto head2tail = obj->get_tail() - obj->get_head();
    // TODO : draw line
    for (int i = 1; i < inter_polating_points_count + 1; i++) {
      // fill push constant
      LinePushConstant push{};
      push.position_ = glm::vec4(head2tail, 0.f);
      push.position_ *= (float)i / (inter_polating_points_count + 1);
      push.position_ += glm::vec4(obj->get_head(), 0.f);
      push.color_ = glm::vec4(obj->get_color(), 0.f);
      push.radius_ = obj->get_radius();

      vkCmdPushConstants(
          frameInfo.commandBuffer_m,
          pipeline_layout_, 
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