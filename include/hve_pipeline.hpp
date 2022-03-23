#pragma once

// std
#include <vector>
#include <string>

namespace hve {

class HvePipeline
{
  public:
    HvePipeline(const std::string& vertFilepath, const std::string& fragFilepath);

  private:
    // fstream can only output char not std::string
    static std::vector<char> readFile(const std::string& filepath);
    void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath);
};

} // namespace hve