// hnll
#include <graphics/pipeline.hpp>
#include <graphics/model.hpp>

// std
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace hnll {

// what kind of geometry will be drawn from the vertices (topoloby) and
// if primitive restart should be enabled
void PipelineConfigInfo::createInputAssemblyInfo()
{
  inputAssemblyInfo_m.sType = 
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyInfo_m.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyInfo_m.primitiveRestartEnable = VK_FALSE;
}

void PipelineConfigInfo::createViewportInfo()
{

  viewportInfo_m.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  // by enabling a GPU feature in logical device creation,
  // its possible to use multiple viewports
  viewportInfo_m.viewportCount = 1;
  viewportInfo_m.pViewports = nullptr;
  viewportInfo_m.scissorCount = 1;
  viewportInfo_m.pScissors = nullptr;
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
  // rasterizationInfo_m.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizationInfo_m.cullMode = VK_CULL_MODE_NONE;
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
  dynamicStateEnables_m = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  dynamicStateInfo_m.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateInfo_m.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables_m.size()); 
  dynamicStateInfo_m.pDynamicStates = dynamicStateEnables_m.data();
  dynamicStateInfo_m.flags = 0;
}


HvePipeline::HvePipeline(
  HveDevice &device,
  const std::string &vertFilepath,
  const std::string &fragFilepath,
  const PipelineConfigInfo &configInfo) : hveDevice_m(device)
{
  createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
}

HvePipeline::~HvePipeline()
{
  vkDestroyShaderModule(hveDevice_m.device(), vertShaderModule_m, nullptr);
  vkDestroyShaderModule(hveDevice_m.device(), fragShaderModule_m, nullptr);
  vkDestroyPipeline(hveDevice_m.device(), graphicsPipeline_m, nullptr);
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

  createShaderModule(vertCode, &vertShaderModule_m);
  createShaderModule(fragCode, &fragShaderModule_m);

  VkPipelineShaderStageCreateInfo shaderStages[2] =
    { createVertShaderStageInfo(), createFragShaderStageInfo() };

  auto vertexInputInfo = createVertexInputInfo();

  // accept vertex data
  auto& bindingDescriptions = configInfo.bindingDescriptions;
  auto& attributeDescriptions = configInfo.attributeDescriptions; 
  vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data(); //optional
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); //optional

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  // programable stage count (in this case vertex and shader stage)
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo_m;
  pipelineInfo.pViewportState = &configInfo.viewportInfo_m;
  pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo_m;
  pipelineInfo.pMultisampleState = &configInfo.multisampleInfo_m;
  pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo_m;
  pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo_m;
  pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo_m;

  pipelineInfo.layout = configInfo.pipelineLayout_m;
  pipelineInfo.renderPass = configInfo.renderPass_m;
  pipelineInfo.subpass = configInfo.subpass_m;

  pipelineInfo.basePipelineIndex = -1;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  // its possible to create multiple VkPipeline objects in a single call
  // second parameter means cache objects enables significantly faster creation
  if (vkCreateGraphicsPipelines(hveDevice_m.device(), VK_NULL_HANDLE, 1,
    &pipelineInfo, nullptr, &graphicsPipeline_m) != VK_SUCCESS)
    throw std::runtime_error("failed to create graphics pipeline!");


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


VkPipelineShaderStageCreateInfo HvePipeline::createVertShaderStageInfo()
{
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule_m;
  // the function to invoke
  vertShaderStageInfo.pName = "main";
  return vertShaderStageInfo;
}

VkPipelineShaderStageCreateInfo HvePipeline::createFragShaderStageInfo()
{
  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule_m;
  // the function to invoke
  fragShaderStageInfo.pName = "main";
  return fragShaderStageInfo;
}

VkPipelineVertexInputStateCreateInfo HvePipeline::createVertexInputInfo()
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // configrate in createGraphicsPipeline()
    // vertexInputInfo.vertexAttributeDescriptionCount = 0;
    // vertexInputInfo.vertexBindingDescriptionCount = 0;
    // vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    // vertexInputInfo.pVertexBindingDescriptions = nullptr;

    return vertexInputInfo;
}

void HvePipeline::bind(VkCommandBuffer commandBuffer)
{
  // basic drawing commands
  // bind the graphics pipeline
  // the second parameter specifies if the pipeline object is a graphics or compute pipeline or ray tracer
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_m);
}

void HvePipeline::defaultPipelineConfigInfo(PipelineConfigInfo &configInfo)
{
  configInfo.createInputAssemblyInfo();
  configInfo.createViewportInfo();
  configInfo.createRasterizationInfo();
  configInfo.createMultisampleState();
  configInfo.createColorBlendAttachment();
  configInfo.createColorBlendState();
  configInfo.createDepthStencilState();
  configInfo.createDynamicState();

  configInfo.bindingDescriptions = HveModel::Vertex::getBindingDescriptions();
  configInfo.attributeDescriptions = HveModel::Vertex::getAttributeDescriptions();
}
} // namespace hve