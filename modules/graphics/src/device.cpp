// hnll
#include <graphics/device.hpp>

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace hnll {

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
device::device(window &window) : window_{window} 
{
  create_instance();
  // window surface should be created right after the instance creation, 
  // because it can actually influence the physical device selection
  setup_debug_messenger();
  create_surface();
  pick_physical_device();
  create_logical_device();
  create_command_pool();
}

device::~device() 
{
  vkDestroyCommandPool(device_, command_pool_, nullptr);
  // VkQueue is automatically destroyed when its device is deleted
  vkDestroyDevice(device_, nullptr);

  if (enable_validation_layers) {
    DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
  }

  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  vkDestroyInstance(instance_, nullptr);
}

// fill in a struct with some informattion about the application
void device::create_instance() 
{
  // validation layers
  if (enable_validation_layers && !check_validation_layer_support()) {
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
  auto extensions = get_required_extensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  // additional debugger for vkCreateInstance and vkDestroyInstance
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (enable_validation_layers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers_.size());
    createInfo.ppEnabledLayerNames = validation_layers_.data();

    populate_debug_messenger_create_info(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } 
  else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  // 1st : pointer to struct with creation info
  // 2nd : pointer to custom allocator callbacks
  // 3rd : pointer to the variable that stores the handle to the new object
  if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS)  
    throw std::runtime_error("failed to create instance!");
  
  has_glfw_required_instance_extensions();
}

void device::pick_physical_device() 
{
  // rate device suitability if its nesessary
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  // allocate an array to hold all of the VkPhysicalDevice handle
  std::cout << "Device count: " << deviceCount << std::endl;
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
  // physical_device_ is constructed as VK_NULL_HANDLE
  for (const auto &device : devices) {
    if (is_device_suitable(device)) {
      physical_device_= device;
      break;
    }
  }

  if (physical_device_ == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  vkGetPhysicalDeviceProperties(physical_device_, &properties);
  std::cout << "physical device: " << properties.deviceName << std::endl;
  swap_chain_support_details details;
  // surface capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &details.capabilities_);
}

void device::create_logical_device() 
{
  queue_family_indices indices = find_queue_families(physical_device_);

  // create a set of all unique queue famililes that are necessary for required queues
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  // if queue families are the same, handle for those queues are also same
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphics_family_.value(), indices.present_family_.value()};

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
  createInfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions_.size());
  createInfo.ppEnabledExtensionNames = device_extensions_.data();

  // might not really be necessary anymore because device specific validation layers
  // have been deprecated
  if (enable_validation_layers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers_.size());
    createInfo.ppEnabledLayerNames = validation_layers_.data();
  } 
  else {
    createInfo.enabledLayerCount = 0;
  }
  // instantiate the logical device
  // logical devices dont interact directly with  instances 
  if (vkCreateDevice(physical_device_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }
  // retrieve queue handles for each queue family
  // simply use index 0, because were only creating a single queue from  this family
  vkGetDeviceQueue(device_, indices.graphics_family_.value(), 0, &graphics_queue_);
  vkGetDeviceQueue(device_, indices.present_family_.value(), 0, &present_queue_);
}

// Command pools manage the memory that is used to store the buffers 
// and com- mand buffers are allocated from them.
void device::create_command_pool() 
{
  queue_family_indices get_queue_family_indices = find_physical_queue_families();

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = get_queue_family_indices.graphics_family_.value();
  poolInfo.flags =
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(device_, &poolInfo, nullptr, &command_pool_) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}

void device::create_surface() { window_.create_window_surface(instance_, &surface_); }

// ensure there is at least one available physical device and
// the debice can present images to the surface we created
bool device::is_device_suitable(VkPhysicalDevice device) 
{
  queue_family_indices indices = find_queue_families(device);

  bool extensionsSupported = check_device_extension_support(device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    swap_chain_support_details swapChainSupport = query_swap_chain_support(device);
    swapChainAdequate = !swapChainSupport.formats_.empty() && !swapChainSupport.present_modes_.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  return indices.is_complete() && extensionsSupported && swapChainAdequate &&
         supportedFeatures.samplerAnisotropy;
}

void device::populate_debug_messenger_create_info(
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
void device::setup_debug_messenger() 
{
  if (!enable_validation_layers) return;
  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populate_debug_messenger_create_info(createInfo);
  if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debug_messenger_) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

bool device::check_validation_layer_support() 
{
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  // check if all of the layers in validation_layers_ exist in the availableLyaerss
  for (const char *layerName : validation_layers_) {
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
std::vector<const char *> device::get_required_extensions() 
{
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  // convert const char** to std::vector<const char*>
  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enable_validation_layers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

void device::has_glfw_required_instance_extensions() 
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
  auto requiredExtensions = get_required_extensions();
  for (const auto &required : requiredExtensions) {
    std::cout << "\t" << required << std::endl;
    if (available.find(required) == available.end()) {
      throw std::runtime_error("Missing required glfw extension");
    }
  }
}

// check for swap chain extension
bool device::check_device_extension_support(VkPhysicalDevice device) 
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

  std::set<std::string> requiredExtensions(device_extensions_.begin(), device_extensions_.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
  // check wheather all glfwExtensions are supported
}

queue_family_indices device::find_queue_families(VkPhysicalDevice device) 
{
  queue_family_indices indices;
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
    // graphics: No DRI3 support detected - required for presentation
    // Note: you can probably enable DRI3 in your Xorg config
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family_ = i;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.present_family_ = i;
    }
    if (indices.is_complete()) {
      break;
    }
    i++;
  }

  queue_family_indices_ = indices;

  return indices;
}

swap_chain_support_details device::query_swap_chain_support(VkPhysicalDevice device) 
{
  swap_chain_support_details details;
  // surface capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities_);
  // surface format list
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats_.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats_.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.present_modes_.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device,
        surface_,
        &presentModeCount,
        details.present_modes_.data());
  }
  return details;
}

VkFormat device::find_supported_format(
    const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
{
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physical_device_, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (
        tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

uint32_t device::find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

void device::create_buffer(
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

  if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  // memory allocation
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }
  // associate the memory with the buffer
  vkBindBufferMemory(device_, buffer, bufferMemory, 0);
}

VkCommandBuffer device::begin_one_shot_commands() 
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = command_pool_;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void device::end_one_shot_commands(VkCommandBuffer commandBuffer)  
{
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(graphics_queue_, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphics_queue_);

  vkFreeCommandBuffers(device_, command_pool_, 1, &commandBuffer);
}

void device::copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
{
  // command buffer for memory transfer operations
  VkCommandBuffer commandBuffer = begin_one_shot_commands();

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;  // Optional
  copyRegion.dstOffset = 0;  // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  end_one_shot_commands(commandBuffer);
}

void device::copy_buffer_to_image(
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) 
{
  VkCommandBuffer commandBuffer = begin_one_shot_commands();

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
  end_one_shot_commands(commandBuffer);
}

void device::create_image_with_info(
    const VkImageCreateInfo &imageInfo,
    VkMemoryPropertyFlags properties,
    VkImage &image,
    VkDeviceMemory &imageMemory) 
{
  if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device_, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS) {
    throw std::runtime_error("failed to bind image memory!");
  }
}

}  // namespace hve