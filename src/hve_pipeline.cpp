#include <hve_pipeline.hpp>

// std
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace hve {

// what kind of geometry will be drawn from the vertices (topoloby) and
// if primitive restart should be enabled
void PipelineConfigInfo::createInputAssemblyInfo()
{
  inputAssemblyInfo_m.sType = 
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyInfo_m.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyInfo_m.primitiveRestartEnable = VK_FALSE;
}

void PipelineConfigInfo::createViewportInfo(uint32_t width, uint32_t height)
{
  // transformation of the image                            
  // draw entire framebuffer
  viewport_m.x = 0.0f;
  viewport_m.y = 0.0f;
  viewport_m.width = static_cast<float>(width);
  viewport_m.height = static_cast<float>(height);
  viewport_m.minDepth = 0.0f;
  viewport_m.maxDepth = 1.0f;
  
  // cut the region of the framebuffer(swap chain)
  scissor_m.offset = {0, 0};
  scissor_m.extent = {width, height};
  
 viewportInfo_m.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  // by enabling a GPU feature in logical device creation,
  // its possible to use multiple viewports
 viewportInfo_m.viewportCount = 1;
 viewportInfo_m.pViewports = &viewport_m;
 viewportInfo_m.scissorCount = 1;
 viewportInfo_m.pScissors = &scissor_m;
}

void PipelineConfigInfo::createRasterizationInfo()
{
  rasterizationInfo_m.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  // using this requires enabling a GPU feature
  rasterizationInfo_m.depthClampEnable = VK_FALSE;
  // if rasterizationInfo_mDiscardEnable is set to VK_TRUE, then geometry never passes
  // through the rasterizationInfo_m stage, basically disables any output to the frame_buffer
  rasterizationInfo_m.rasterizerDiscardEnable = VK_FALSE;
  // how fragments are generated for geometry
  // using any mode other than fill requires GPU feature
  rasterizationInfo_m.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationInfo_m.lineWidth = 1.0f;
  rasterizationInfo_m.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizationInfo_m.frontFace = VK_FRONT_FACE_CLOCKWISE;
  // consider this when shadow mapping is necessary
  rasterizationInfo_m.depthBiasEnable = VK_FALSE;
  rasterizationInfo_m.depthBiasConstantFactor = 0.0f;
  rasterizationInfo_m.depthBiasClamp = 0.0f;
  rasterizationInfo_m.depthBiasSlopeFactor = 0.0f;
}

// used for anti-aliasing
void PipelineConfigInfo::createMultisampleState()
{
  multisampleInfo_m.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleInfo_m.sampleShadingEnable = VK_FALSE;
  multisampleInfo_m.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleInfo_m.minSampleShading = 1.0f;
  multisampleInfo_m.pSampleMask = nullptr;
  multisampleInfo_m.alphaToCoverageEnable = VK_FALSE;
  multisampleInfo_m.alphaToOneEnable = VK_FALSE;
}

// color blending for alpha blending
void PipelineConfigInfo::createColorBlendAttachment()
{
  // per framebuffer struct
  // in contrast, VkPipelineColcorBlendStateCreateInfo is global color blending settings
  colorBlendAttachment_m.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; 
  colorBlendAttachment_m.blendEnable = VK_FALSE;
  colorBlendAttachment_m.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  colorBlendAttachment_m.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment_m.colorBlendOp = VK_BLEND_OP_ADD; // Optional 
  colorBlendAttachment_m.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; //Optional
  colorBlendAttachment_m.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; //Optional
  colorBlendAttachment_m.alphaBlendOp = VK_BLEND_OP_ADD; 
  colorBlendAttachment_m.blendEnable = VK_TRUE;
  colorBlendAttachment_m.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; 
  colorBlendAttachment_m.dstColorBlendFactor =VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; 
  colorBlendAttachment_m.colorBlendOp = VK_BLEND_OP_ADD; 
  colorBlendAttachment_m.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; 
  colorBlendAttachment_m.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment_m.alphaBlendOp = VK_BLEND_OP_ADD;
}

void PipelineConfigInfo::createColorBlendState()
{
  colorBlendInfo_m.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlendInfo_m.logicOpEnable = VK_FALSE; 
  colorBlendInfo_m.logicOp = VK_LOGIC_OP_COPY; // Optional 
  colorBlendInfo_m.attachmentCount = 1; 
  colorBlendInfo_m.pAttachments = &colorBlendAttachment_m; 
  colorBlendInfo_m.blendConstants[0] = 0.0f; // Optional 
  colorBlendInfo_m.blendConstants[1] = 0.0f; // Optional
  colorBlendInfo_m.blendConstants[2] = 0.0f; // Optional 
  colorBlendInfo_m.blendConstants[3] = 0.0f; // Optional
}

void PipelineConfigInfo::createDepthStencilState()
{
  depthStencilInfo_m.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencilInfo_m.depthTestEnable = VK_TRUE;
  depthStencilInfo_m.depthWriteEnable = VK_TRUE;
  depthStencilInfo_m.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencilInfo_m.depthBoundsTestEnable = VK_FALSE;
  depthStencilInfo_m.minDepthBounds = 0.0f;  // Optional
  depthStencilInfo_m.maxDepthBounds = 1.0f;  // Optional
  depthStencilInfo_m.stencilTestEnable = VK_FALSE;
  depthStencilInfo_m.front = {};  // Optional
  depthStencilInfo_m.back = {};   // Optional
}

// a limited amount of the state can be actually be changed without recreating the pipeline
void PipelineConfigInfo::createDynamicState()
{
  VkDynamicState dynamicStates[] = {};
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 0; 
  dynamicState.pDynamicStates = nullptr;
}


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

  configInfo.createInputAssemblyInfo();
  configInfo.createViewportInfo(width, height);
  configInfo.createRasterizationInfo();
  configInfo.createMultisampleState();
  configInfo.createColorBlendAttachment();
  configInfo.createColorBlendState();
  configInfo.createDepthStencilState();

  return configInfo;
}
} // namespace hve