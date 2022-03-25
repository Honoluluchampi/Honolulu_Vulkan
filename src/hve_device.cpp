#include <hve_device.hpp>

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace hve {

// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) 
{
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) 
{
  // create the extension object if its available
  // Since the debug messenger is specific to our Vulkan instance and its layers, it needs to be explicitly specified as first argument.
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance,
      "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) 
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance,
      "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

// class member functions
HveDevice::HveDevice(HveWindow &window) : window_m{window} 
{
  createInstance();
  // window surface should be created right after the instance creation, 
  // because it can actually influence the physical device selection
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createCommandPool();
}

HveDevice::~HveDevice() 
{
  vkDestroyCommandPool(device_m, commandPool_m, nullptr);
  // VkQueue is automatically destroyed when its device is deleted
  vkDestroyDevice(device_m, nullptr);

  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(instance_m, debugMessenger_m, nullptr);
  }

  vkDestroySurfaceKHR(instance_m, surface_m, nullptr);
  vkDestroyInstance(instance_m, nullptr);
}

// fill in a struct with some informattion about the application
void HveDevice::createInstance() 
{
  // validation layers
  if (enableValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "HonoluluVulkanEngine App";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  // getting required extentions according to whether debug mode or not
  // glfw extensions are configured in somewhere else
  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  // additional debugger for vkCreateInstance and vkDestroyInstance
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } 
  else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  // 1st : pointer to struct with creation info
  // 2nd : pointer to custom allocator callbacks
  // 3rd : pointer to the variable that stores the handle to the new object
  if (vkCreateInstance(&createInfo, nullptr, &instance_m) != VK_SUCCESS)  
    throw std::runtime_error("failed to create instance!");
  
  hasGflwRequiredInstanceExtensions();
}

void HveDevice::pickPhysicalDevice() 
{
  // rate device suitability if its nesessary
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance_m, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  // allocate an array to hold all of the VkPhysicalDevice handle
  std::cout << "Device count: " << deviceCount << std::endl;
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance_m, &deviceCount, devices.data());
  // physicalDevice_m is constructed as VK_NULL_HANDLE
  for (const auto &device : devices) {
    if (isDeviceSuitable(device)) {
      physicalDevice_m= device;
      break;
    }
  }

  if (physicalDevice_m == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  vkGetPhysicalDeviceProperties(physicalDevice_m, &properties_m);
  std::cout << "physical device: " << properties_m.deviceName << std::endl;
  SwapChainSupportDetails details;
  // surface capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_m, surface_m, &details.capabilities_m);
  std::cout << details.capabilities_m.maxImageExtent.height << std::endl;
  std::cout << details.capabilities_m.minImageExtent.height << std::endl;
}

void HveDevice::createLogicalDevice() 
{
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice_m);

  // create a set of all unique queue famililes that are necessary for required queues
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  // if queue families are the same, handle for those queues are also same
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily_m.value(), indices.presentFamily_m.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    // VkDeviceQueueCreateInfo descrives the number of queues we want for a single queue family
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    // Vulkan lets us assign priorities to queues to influence the scheduling of commmand buffer execut9on
    // using floating point numbers between 0.0 and 1.0
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }
  // we need nothing special right now
  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  // filling in the main VkDeviceCreateInfo structure;
  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;
  // enable device extension 
  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  // might not really be necessary anymore because device specific validation layers
  // have been deprecated
  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } 
  else {
    createInfo.enabledLayerCount = 0;
  }
  // instantiate the logical device
  // logical devices dont interact directly with  instances 
  if (vkCreateDevice(physicalDevice_m, &createInfo, nullptr, &device_m) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }
  // retrieve queue handles for each queue family
  // simply use index 0, because were only creating a single queue from  this family
  vkGetDeviceQueue(device_m, indices.graphicsFamily_m.value(), 0, &graphicsQueue_m);
  vkGetDeviceQueue(device_m, indices.presentFamily_m.value(), 0, &presentQueue_m);
}

// Command pools manage the memory that is used to store the buffers 
// and com- mand buffers are allocated from them.
void HveDevice::createCommandPool() 
{
  QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily_m.value();
  poolInfo.flags =
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(device_m, &poolInfo, nullptr, &commandPool_m) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}

void HveDevice::createSurface() { window_m.createWindowSurface(instance_m, &surface_m); }

// ensure there is at least one available physical device and
// the debice can present images to the surface we created
bool HveDevice::isDeviceSuitable(VkPhysicalDevice device) 
{
  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats_m.empty() && !swapChainSupport.presentModes_m.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  return indices.isComplete() && extensionsSupported && swapChainAdequate &&
         supportedFeatures.samplerAnisotropy;
}

void HveDevice::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) 
{
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr;  // Optional
}

// fix this function to control debug call back of the apps
void HveDevice::setupDebugMessenger() 
{
  if (!enableValidationLayers) return;
  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);
  if (CreateDebugUtilsMessengerEXT(instance_m, &createInfo, nullptr, &debugMessenger_m) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

bool HveDevice::checkValidationLayerSupport() 
{
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  // check if all of the layers in validationLayers exist in the availableLyaerss
  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

// required list of extensions based on wheather validation lyaers are enabled
std::vector<const char *> HveDevice::getRequiredExtensions() 
{
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  // convert const char** to std::vector<const char*>
  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

void HveDevice::hasGflwRequiredInstanceExtensions() 
{
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

  std::cout << "available extensions:" << std::endl;
  std::unordered_set<std::string> available;
  for (const auto &extension : extensions) {
    std::cout << "\t" << extension.extensionName << std::endl;
    available.insert(extension.extensionName);
  }

  std::cout << "required extensions:" << std::endl;
  auto requiredExtensions = getRequiredExtensions();
  for (const auto &required : requiredExtensions) {
    std::cout << "\t" << required << std::endl;
    if (available.find(required) == available.end()) {
      throw std::runtime_error("Missing required glfw extension");
    }
  }
}

// check for swap chain extension
bool HveDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) 
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
  // each VkExtensionProperties contains the name and version of an extension
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(
      device,
      nullptr,
      &extensionCount,
      availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
  // check wheather all glfwExtensions are supported
}

QueueFamilyIndices HveDevice::findQueueFamilies(VkPhysicalDevice device) 
{
  QueueFamilyIndices indices;
  // retrieve the list of quque families 
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  // vkQueueFamilyProperties struct contains some details about the queue family
  // the type of operations, the number of queue that can be created based on that familyo
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  // check whether at least one queueFamily support VK_QUEUEGRAPHICS_BIT
  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    // same i for presentFamily and graphicsFamily improves the performance
    // vulkan: No DRI3 support detected - required for presentation
    // Note: you can probably enable DRI3 in your Xorg config
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily_m = i;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_m, &presentSupport);
    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.presentFamily_m = i;
    }
    if (indices.isComplete()) {
      break;
    }
    i++;
  }

  return indices;
}

SwapChainSupportDetails HveDevice::querySwapChainSupport(VkPhysicalDevice device) 
{
  SwapChainSupportDetails details;
  // surface capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_m, &details.capabilities_m);
  // surface format list
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_m, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats_m.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_m, &formatCount, details.formats_m.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_m, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes_m.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device,
        surface_m,
        &presentModeCount,
        details.presentModes_m.data());
  }
  return details;
}

VkFormat HveDevice::findSupportedFormat(
    const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
{
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice_m, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (
        tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

uint32_t HveDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice_m, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

void HveDevice::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer &buffer,
    VkDeviceMemory &bufferMemory)
{
  // buffer creation
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device_m, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  // memory allocation
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device_m, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device_m, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }
  // associate the memory with the buffer
  vkBindBufferMemory(device_m, buffer, bufferMemory, 0);
}

VkCommandBuffer HveDevice::beginSingleTimeCommands() 
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool_m;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device_m, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void HveDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer)  
{
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(graphicsQueue_m, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue_m);

  vkFreeCommandBuffers(device_m, commandPool_m, 1, &commandBuffer);
}

void HveDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
{
  // command buffer for memory transfer operations
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;  // Optional
  copyRegion.dstOffset = 0;  // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer);
}

void HveDevice::copyBufferToImage(
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) 
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = layerCount;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(
      commandBuffer,
      buffer,
      image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region);
  endSingleTimeCommands(commandBuffer);
}

void HveDevice::createImageWithInfo(
    const VkImageCreateInfo &imageInfo,
    VkMemoryPropertyFlags properties,
    VkImage &image,
    VkDeviceMemory &imageMemory) 
{
  if (vkCreateImage(device_m, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device_m, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device_m, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  if (vkBindImageMemory(device_m, image, imageMemory, 0) != VK_SUCCESS) {
    throw std::runtime_error("failed to bind image memory!");
  }
}

}  // namespace hve