#include <hve_pipeline.hpp>

// std
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace hve {

  HvePipeline::HvePipeline(const std::string& vertFilepath, const std::string& fragFilepath)
  { createGraphicsPipeline(vertFilepath, fragFilepath); }

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

  void HvePipeline::createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath)
  {
    auto vertCode = readFile(vertFilepath);
    auto fragCode = readFile(fragFilepath);

    std::cout << "Vertex Shader Code Size: " << vertCode.size() << '\n';
    std::cout << "Fragment Shader Code Size: " << fragCode.size() << '\n';
  }

} // namespace hve