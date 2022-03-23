#pragma once

#include <hve_device.hpp>

// std
#include <vector>
#include <string>

namespace hve
{

  struct PipelineConfigInfo
  {
  };

  class HvePipeline
  {
  public:
    HvePipeline(
        HveDevice &device,
        const std::string &vertFilepath,
        const std::string &fragFilepath,
        const PipelineConfigInfo &configInfo);
    ~HvePipeline() {}

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
  
    // 
    HveDevice& hveDevice_m;
    VkPipeline graphicsPipeline_m;
    VkShaderModule vertShaderModule_m;
    VkShaderModule fragShaderModule_m;
  };

} // namespace hve