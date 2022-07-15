#pragma once

// hnll
#include <graphics/device.hpp>

// std
#include <vector>
#include <string>

namespace hnll {

struct PipelineConfigInfo 
{
  PipelineConfigInfo() = default;
  PipelineConfigInfo(const PipelineConfigInfo&) = delete;
  PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
  // for pipeline config info
  void createInputAssemblyInfo();
  // viewport and scissor
  void createViewportInfo();
  // rasterizer
  void createRasterizationInfo();
  // multisampling used for anti-aliasing
  void createMultisampleState();
  // color blending for alpha blending
  void createColorBlendAttachment();
  void createColorBlendState();
  void createDepthStencilState();
  // dynamic state
  void createDynamicState();

  // member variables
  std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo_m{};
  VkPipelineViewportStateCreateInfo viewportInfo_m{};
  VkPipelineRasterizationStateCreateInfo rasterizationInfo_m{};
  VkPipelineMultisampleStateCreateInfo multisampleInfo_m{};
  VkPipelineColorBlendAttachmentState colorBlendAttachment_m{};
  VkPipelineColorBlendStateCreateInfo colorBlendInfo_m{};
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo_m{};
  std::vector<VkDynamicState> dynamicStateEnables_m;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo_m{};
  VkPipelineLayout pipelineLayout_m = nullptr;
  VkRenderPass renderPass_m = nullptr;
  uint32_t subpass_m = 0;
};

class HvePipeline
{
  public:
    HvePipeline(
        device &device,
        const std::string &vertFilepath,
        const std::string &fragFilepath,
        const PipelineConfigInfo &configInfo);
    ~HvePipeline();

    // uncopyable
    HvePipeline(const HvePipeline &) = delete;
    HvePipeline& operator=(const HvePipeline &) = delete;

    void bind(VkCommandBuffer commandBuffer);

    static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

  private:
    // fstream can only output char not std::string
    static std::vector<char> readFile(const std::string &filepath);

    void createGraphicsPipeline(
      const std::string &vertFilepath, 
      const std::string &fragFilepath, 
      const PipelineConfigInfo &configInfo);

    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    VkPipelineShaderStageCreateInfo createVertShaderStageInfo();
    VkPipelineShaderStageCreateInfo createFragShaderStageInfo();
    VkPipelineVertexInputStateCreateInfo createVertexInputInfo();
    // 
    device& hveDevice_m;
    VkPipeline graphicsPipeline_m;
    VkShaderModule vertShaderModule_m;
    VkShaderModule fragShaderModule_m;
};

} // namespace hve