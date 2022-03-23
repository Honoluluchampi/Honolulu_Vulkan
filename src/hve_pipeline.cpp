#include <hve_pipeline.hpp>

// std
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace hve {

  HvePipeline::HvePipeline(
    HveDevice &device,
    const std::string &vertFilepath,
    const std::string &fragFilepath,
    const PipelineConfigInfo &configInfo) : hveDevice_m(device)
  {
    createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
  }

  std::vector<char> HvePipeline::readFile(const std::string& filepath)
  {
    // construct and open
    // immidiately read as binary
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
      throw std::runtime_error("failed to open file: " + filepath);

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
  }

  void HvePipeline::createGraphicsPipeline(
      const std::string &vertFilepath, 
      const std::string &fragFilepath, 
      const PipelineConfigInfo &configInfo)
  {
    auto vertCode = readFile(vertFilepath);
    auto fragCode = readFile(fragFilepath);

    std::cout << "Vertex Shader Code Size: " << vertCode.size() << '\n';
    std::cout << "Fragment Shader Code Size: " << fragCode.size() << '\n';
  }

  void HvePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
  {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    // char to uint32_t
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(hveDevice_m.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
      throw std::runtime_error("failed to create shader module!");
  } 

  PipelineConfigInfo HvePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height)
  {
    PipelineConfigInfo configInfo{};

    return configInfo;
  }
} // namespace hve