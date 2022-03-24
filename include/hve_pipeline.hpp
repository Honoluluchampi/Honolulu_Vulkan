#pragma once

#include <hve_device.hpp>

// std
#include <vector>
#include <string>

namespace hve
{

struct PipelineConfigInfo {
  // for pipeline config info
  void createInputAssemblyInfo();
  // viewport and scissor
  void createViewportScissor(uint32_t width, uint32_t height);
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
  VkViewport viewport_m;
  VkRect2D scissor_m;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo_m;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo_m;
  VkPipelineMultisampleStateCreateInfo multisampleInfo_m;
  VkPipelineColorBlendAttachmentState colorBlendAttachment_m;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo_m;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo_m;
  VkPipelineLayout pipelineLayout_m = nullptr;
  VkRenderPass renderPass_m = nullptr;
  uint32_t subpass_m = 0;
};

class HvePipeline
{
  public:
    HvePipeline(
        HveDevice &device,
        const std::string &vertFilepath,
        const std::string &fragFilepath,
        const PipelineConfigInfo &configInfo);
    ~HvePipeline();

    // uncopyable
    HvePipeline(const HvePipeline &) = delete;
    void operator=(const HvePipeline &) = delete;

    static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

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
    HveDevice& hveDevice_m;
    VkPipeline graphicsPipeline_m;
    VkShaderModule vertShaderModule_m;
    VkShaderModule fragShaderModule_m;
};

} // namespace hve