#pragma once
#include <SDL2/SDL_vulkan.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <utils.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan_initializers.hpp>
#include <cstring>

// for loading stb image function objs
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_STATIC
#include <stb_image.h>

namespace VulkanHelper {

//===================================================
// Misc Helper functions
//===================================================
inline bool
iCheckValidationLayerSupport(std::vector<const char *> validationLayers) {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (int i = 0; i < validationLayers.size(); i++) {

    bool layerFound = false;

    // std::cout << "Layer :" << validationLayers[i] << "\n";

    for (int j = 0; j < availableLayers.size(); j++) {

      // std::cout << "Avail Layer :" << availableLayers[j].layerName << "\n";

      //== compairs pointers
      // strcmp compairs actual string content
      if (strcmp(validationLayers[i], availableLayers[j].layerName)) {
        layerFound = true;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

inline std::vector<const char *> iGetRequiredVkExtensions(SDL_Window *window) {

  unsigned int extensionCount;

  if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr)) {
    std::cout << "Can't get required extensions\n";
  }
  // Need to allocate memory according to required extensions
  // This is different than glfw which returns a pointer with extension memory
  // already allocated
  std::vector<const char *> extensions;
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  size_t additional_extension_count = extensions.size();
  extensions.resize(additional_extension_count + extensionCount);

  // Array needs to be prefilled with available extension names
  if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount,
                                        extensions.data() +
                                            additional_extension_count)) {
    std::cout << "Extensions Didn't get it right\n";
  }

  for (int i = 0; i < extensions.size(); i++) {
    std::cout << "ext: " << extensions[i] << "\n";
  }

  return extensions;
}

inline Utils::QueueFamilyIndices iFindQueueFamilies(VkPhysicalDevice device,
                                                    VkSurfaceKHR surface) {
  Utils::QueueFamilyIndices indices;
  indices.graphicsFamily = -1;
  indices.presentFamily = -1;

  // Queues are what you submit command buffers to, and a queue family describes
  // a set of queues that do a certain thing e.g. graphics for draw calls
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  bool foundPresentSupport = false;

  for (uint32_t i = 0; i < queueFamilies.size(); i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }

    // Find if the device supports window system and present images to the
    // surface we created
    VkBool32 presentSupport = false;

    // To determine whether a queue family of a physical device supports
    // presentation to a given surface
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    if (presentSupport && !foundPresentSupport) {
      indices.presentFamily = i;
      foundPresentSupport = true;
    }
  }

  std::cout << "graphicsFamily: " << indices.graphicsFamily
            << " presentFamily: " << indices.presentFamily << "\n";

  if (indices.graphicsFamily == -1 || indices.presentFamily == -1) {
    throw std::runtime_error("Issue with iFindQueueFamilies");
  }

  return indices;
}

inline Utils::SwapChainSupportDetails
iQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
  Utils::SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         details.formats.data());
  } else {
    std::cout << "Swapchain Support, has no formats";
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                            nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount, details.presentModes.data());
  } else {
    std::cout << "Swapchain Support, has no present modes";
  }
  return details;
}

inline bool
iCheckDeviceExtensionSupport(VkPhysicalDevice device,
                             std::vector<const char *> deviceExtensions) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (int i = 0; i < availableExtensions.size(); i++) {
    // std::cout << "Avail: " << availableExtensions[i].extensionName
    //           << " Version: " << availableExtensions[i].specVersion << "\n";
    requiredExtensions.erase(availableExtensions[i].extensionName);
  }
  if (requiredExtensions.empty()) {
    std::cout << "Physical device contains all required extensions\n";
  }

  return requiredExtensions.empty();
}

inline bool iIsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
                              std::vector<const char *> deviceExtensions) {

  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceFeatures deviceFeatures;

  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  std::cout << "Device Name: " << deviceProperties.deviceName
            << " Device ID: " << deviceProperties.deviceID << "\n";

  Utils::QueueFamilyIndices indices = iFindQueueFamilies(device, surface);

  bool swapChainAdequate = false;
  Utils::SwapChainSupportDetails swapChainSupport =
      iQuerySwapChainSupport(device, surface);

  swapChainAdequate = !swapChainSupport.formats.empty() &&
                      !swapChainSupport.presentModes.empty();

  return indices.graphicsFamily != -1 && indices.presentFamily != -1 &&
         iCheckDeviceExtensionSupport(device, deviceExtensions) &&
         swapChainAdequate && deviceFeatures.samplerAnisotropy;
}
inline VkSurfaceFormatKHR iChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {

  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  // Else just settle on the first format
  return availableFormats[0];
}

inline VkPresentModeKHR iChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

// Extent is resolution of swap chain images which is based on surface

// capabilities currentExtent is the current width and height of the surface, or
// the special value (0xFFFFFFFF, 0xFFFFFFFF) indicating that the surface size
// will be determined by the extent of a swapchain targeting the surface.
inline VkExtent2D
iChooseSwapExtent(SDL_Window *window,
                  const VkSurfaceCapabilitiesKHR &capabilities) {

  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {

    std::cout << "Swapchain W:" << capabilities.currentExtent.width << " H:" << capabilities.currentExtent.height << "\n";
    return capabilities.currentExtent;
  } else {
    int width, height;
    // glfwGetFramebufferSize(window, &width, &height);
    SDL_Vulkan_GetDrawableSize(window, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    std::cout << "Swapchain W:" << actualExtent.width << " H:" << actualExtent.height << "\n";

    return actualExtent;
  }
}

inline VkImageView createImageView(VkDevice device, VkImage image,
                                   VkFormat format,
                                   VkImageAspectFlags aspectFlags) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;

  // The viewType and format fields specify how the image data should be
  // interpreted. The viewType parameter allows you to treat images as 1D
  // textures, 2D textures, 3D textures and cube maps.
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;

  // allows you to swizzle the color channels around. For example, you can map
  // all of the channels to the red channel for a monochrome texture. You can
  // also map constant values of 0 and 1 to a channel.
  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  // subresourceRange field describes what the image's purpose is and which
  // part of the image should be accessed. Our images will be used as color
  // targets without any mipmapping levels or multiple layers.
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }

  return imageView;
}

inline std::vector<char> readFile(std::string filePath) {

  // std::ios::ate means seek the end immediatly
  // std::ios::binary read it in as a binary
  std::ifstream file{filePath, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filePath);
  }

  // tellg gets last position which is the filesize
  size_t fileSize = static_cast<size_t>(file.tellg());

  std::vector<char> buffer(fileSize);

  // Go to beginning
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

inline VkShaderModule createShaderModule(VkDevice logicalDevice,
                                         const std::vector<char> &code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }
  return shaderModule;
}

inline VkPipelineShaderStageCreateInfo loadShader(VkDevice logicalDevice, const std::vector<char> &code, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = createShaderModule(logicalDevice, code);
	shaderStage.pName = "main";
	assert(shaderStage.module != VK_NULL_HANDLE);
	return shaderStage;
}

inline VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice,
                                    const std::vector<VkFormat> &candidates,
                                    VkImageTiling tiling,
                                    VkFormatFeatureFlags features) {

  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

inline uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
                               uint32_t typeFilter,
                               VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;

  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    // typeFiler specifies the bit field of memory types that are suitable
    // can find index of suitable memory by iterating over all memoryTypes and
    // checking if the bit is set to 1

    // need to also look at special features of the memory, like being able to
    // map so we can write to it from CPU so look for a bitwise match

    if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags &
                                  properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("Failed to find memory type!");
}

inline void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device,
                         VkDeviceSize size, VkBufferUsageFlags usage,
                         VkMemoryPropertyFlags properties, VkBuffer &buffer,
                         VkDeviceMemory &bufferMemory) {

  VkBufferCreateInfo bufferInfo = VulkanInit::buffer_create_info(usage, size);
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }

  // After this buffer has been created, but doesn't have memory inside
  // First step of allocating memory to buffer requires querying its memory
  // requirements
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = VulkanInit::memory_allocate_info();
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(
      physicalDevice, memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }
  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

inline VkCommandBuffer beginSingleTimeCommands(VkDevice device,
                                               VkCommandPool commandPool) {
  // First need to allocate a temporary command buffer
  //  Can create seperate command pool, but maybe at another time

  VkCommandBufferAllocateInfo allocInfo =
      VulkanInit::command_buffer_allocate_info(
          commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

  // Begin recording to command buffer
  VkCommandBufferBeginInfo beginInfo = VulkanInit::command_buffer_begin_info();

  // telling driver about our onetime usage
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

inline void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool,
                                  VkCommandBuffer commandBuffer,
                                  VkQueue submitQueue) {

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FLAGS_NONE;

  VkFence fence;
	VK_CHECK(vkCreateFence(device, &fence_info, nullptr, &fence), "vkCreateFence");

  vkQueueSubmit(submitQueue, 1, &submitInfo, fence);

  // Wait for the fence to signal that command buffer has finished executing
	VK_CHECK(vkWaitForFences(device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT), "vkWaitForFences");

  vkDestroyFence(device, fence, nullptr);

  vkQueueWaitIdle(submitQueue);

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

inline void beginDrawingCommandBuffer(VkCommandBuffer commandBuffer) {

  if (vkResetCommandBuffer(commandBuffer, 0) != VK_SUCCESS) {
    throw std::runtime_error("failed to reset command buffer!");
  }

  VkCommandBufferBeginInfo beginInfo = VulkanInit::command_buffer_begin_info();

  // telling driver about our onetime usage
  //beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

inline void endDrawingCommandBuffer(VkCommandBuffer commandBuffer,
                                    VkQueue submitQueue,
                                    VkSemaphore imageAvailableSemaphore,
                                    VkSemaphore renderFinishedSemaphore,
                                    VkFence inFlightFence) {
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record drawing command buffer!");
  }
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  VK_CHECK(vkQueueSubmit(submitQueue, 1, &submitInfo, inFlightFence), "endDrawingCommandBuffer");
}

inline void submitCommandBuffers(std::vector<VkCommandBuffer> commandBuffers,
                                    VkQueue submitQueue,
                                    VkSemaphore imageAvailableSemaphore,
                                    VkSemaphore renderFinishedSemaphore,
                                    VkFence inFlightFence) {
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;


  submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
	submitInfo.pCommandBuffers = commandBuffers.data();


  VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  VK_CHECK(vkQueueSubmit(submitQueue, 1, &submitInfo, inFlightFence), "submitCommandBuffers");
}

inline void copyBuffer(VkDevice device, VkCommandPool commandPool,
                       VkQueue submitQueue, VkBuffer srcBuffer,
                       VkBuffer dstBuffer, VkDeviceSize size) {

  VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(device, commandPool, commandBuffer, submitQueue);
}

inline bool hasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

inline void transitionImageLayout(VkDevice device, VkCommandPool commandPool,
                                  VkQueue submitQueue, VkImage image,
                                  VkFormat format, VkImageLayout oldLayout,
                                  VkImageLayout newLayout) {

  VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;

  // must be set to VK_QUEUE_FAMILY_IGNORED if you don't want to transfer queue
  // family ownership
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  barrier.srcAccessMask = 0; // TODO
  barrier.dstAccessMask = 0; // TODO

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  // For depth image transitions
  if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (hasStencilComponent(format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }
  ////////

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

  } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  }

  else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);
  endSingleTimeCommands(device, commandPool, commandBuffer, submitQueue);
}

// Utils::Texture loadTexture(const char *texPath, VkFormat format,
//                            VkPhysicalDevice physicalDevice, VkDevice device,
//                            VkCommandPool commandPool, VkQueue submitQueue);

inline Utils::Texture loadTexture(const char *texPath, VkFormat format,
                        VkPhysicalDevice physicalDevice,
                        VkDevice device, 
                        VkCommandPool commandPool,
                        VkQueue submitQueue) {
  Utils::Texture texture{};

  texture.texPath = texPath;

  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(texPath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    throw std::runtime_error("failed to load texture image: " + std::string(texPath));
  }

  texture.height = texHeight;
  texture.width = texWidth;
  texture.mip_levels = 1;

  // Now copy raw image data to an optimal tiled image
  // This loads the texture data into a host local buffer that is copied to the optimal tiled image on the device

  // Create a host-visible staging buffer that contains the raw image data
  // This buffer will be the data source for copying texture data to the optimal tiled image on the device
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  createBuffer(physicalDevice, device, imageSize,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);
  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(device, stagingBufferMemory);

  stbi_image_free(pixels);

  // Create optimal tiled target image on the device
	VkImageCreateInfo image_create_info = VulkanInit::image_create_info();
	image_create_info.imageType         = VK_IMAGE_TYPE_2D;
	image_create_info.format            = format;
	image_create_info.mipLevels         = texture.mip_levels;
	image_create_info.arrayLayers       = 1;
	image_create_info.samples           = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling            = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
	// Set initial layout of the image to undefined
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.extent        = {texture.width, texture.height, 1};
	image_create_info.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VK_CHECK(vkCreateImage(device, &image_create_info, nullptr, &texture.image), "vkCreateImage");

  VkMemoryAllocateInfo memory_allocate_info = VulkanInit::memory_allocate_info();
  VkMemoryRequirements memory_requirements  = {};
  vkGetImageMemoryRequirements(device, texture.image, &memory_requirements);
	memory_allocate_info.allocationSize  = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = findMemoryType(physicalDevice, 
                                                        memory_requirements.memoryTypeBits, 
                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK(vkAllocateMemory(device, &memory_allocate_info, nullptr, &texture.device_memory), "vkAllocateMemory");
	VK_CHECK(vkBindImageMemory(device, texture.image, texture.device_memory, 0), "vkBindImageMemory");


  VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

  //Now transition image layout from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL

  // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
	VkImageSubresourceRange subresource_range = {};
	// Image only contains color data
	subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// Start at first mip level
	subresource_range.baseMipLevel = 0;
	// We will transition on all mip levels
	subresource_range.levelCount = texture.mip_levels;
	// The 2D texture only has one layer
	subresource_range.layerCount = 1;


  // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
	VkImageMemoryBarrier image_memory_barrier = VulkanInit::image_memory_barrier();

	image_memory_barrier.image            = texture.image;
	image_memory_barrier.subresourceRange = subresource_range;
	image_memory_barrier.srcAccessMask    = 0;
	image_memory_barrier.dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;
	image_memory_barrier.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
	image_memory_barrier.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
	// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
	// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
	vkCmdPipelineBarrier(
	    commandBuffer,
	    VK_PIPELINE_STAGE_HOST_BIT,
	    VK_PIPELINE_STAGE_TRANSFER_BIT,
	    0,
	    0, nullptr,
	    0, nullptr,
	    1, &image_memory_barrier);

  VkBufferImageCopy buffer_copy_region               = {};
			buffer_copy_region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			buffer_copy_region.imageSubresource.mipLevel       = 0;
			buffer_copy_region.imageSubresource.baseArrayLayer = 0;
			buffer_copy_region.imageSubresource.layerCount     = 1;
			buffer_copy_region.imageExtent.width               = texture.width;
			buffer_copy_region.imageExtent.height              = texture.height;
			buffer_copy_region.imageExtent.depth               = 1;
			buffer_copy_region.bufferOffset                    = 0;

  vkCmdCopyBufferToImage(
		    commandBuffer,
		    stagingBuffer,
		    texture.image,
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		    1,
		    &buffer_copy_region);

  // Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
	image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	image_memory_barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	image_memory_barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
	// Source pipeline stage stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
	// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
	vkCmdPipelineBarrier(
		    commandBuffer,
		    VK_PIPELINE_STAGE_TRANSFER_BIT,
		    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		    0,
		    0, nullptr,
		    0, nullptr,
		    1, &image_memory_barrier);
  
  // Store current layout for later reuse
	texture.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  endSingleTimeCommands(device, commandPool, commandBuffer, submitQueue);

  // Clean up staging resources
	vkFreeMemory(device, stagingBufferMemory, nullptr);
	vkDestroyBuffer(device, stagingBuffer, nullptr);

  // Create a texture sampler
	// In Vulkan textures are accessed by samplers
	// This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
	// Note: Similar to the samplers available with OpenGL 3.3
	VkSamplerCreateInfo sampler = VulkanInit::sampler_create_info();
	sampler.magFilter           = VK_FILTER_LINEAR;
	sampler.minFilter           = VK_FILTER_LINEAR;
	sampler.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeV        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeW        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.mipLodBias          = 0.0f;
	sampler.compareOp           = VK_COMPARE_OP_NEVER;
	sampler.minLod              = 0.0f;
	// Set max level-of-detail to mip level count of the texture
	sampler.maxLod = static_cast<float>(texture.mip_levels);
	// Enable anisotropic filtering
	// This feature is optional, so we must check if it's supported on the device

  VkPhysicalDeviceFeatures features{};
  vkGetPhysicalDeviceFeatures(physicalDevice, &features);
	if (features.samplerAnisotropy)
	{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		// Use max. level of anisotropy for this example
		sampler.maxAnisotropy    = properties.limits.maxSamplerAnisotropy;
		sampler.anisotropyEnable = VK_TRUE;
	}
	else
	{
		// The device does not support anisotropic filtering
		sampler.maxAnisotropy    = 1.0;
		sampler.anisotropyEnable = VK_FALSE;
	}
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK(vkCreateSampler(device, &sampler, nullptr, &texture.sampler), "vkCreateSampler");

  // Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges
  VkImageViewCreateInfo view = VulkanInit::image_view_create_info();
	view.viewType              = VK_IMAGE_VIEW_TYPE_2D;
	view.format                = format;
	view.components            = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
	// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
	// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
	view.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	view.subresourceRange.baseMipLevel   = 0;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount     = 1;
	// Linear tiling usually won't support mip maps
	// Only set mip map count if optimal tiling is used
	view.subresourceRange.levelCount = texture.mip_levels;
	// The view will be based on the texture's image
	view.image = texture.image;
	VK_CHECK(vkCreateImageView(device, &view, nullptr, &texture.view), "vkCreateImageView");

  return texture;
}

inline void setImageLayout(
  VkCommandBuffer cmdbuffer,
  VkImage image,
  VkImageLayout oldImageLayout,
  VkImageLayout newImageLayout,
  VkImageSubresourceRange subresourceRange,
  VkPipelineStageFlags srcStageMask,
  VkPipelineStageFlags dstStageMask)
{
  // Create an image barrier object
  VkImageMemoryBarrier imageMemoryBarrier = VulkanInit::image_memory_barrier();
  imageMemoryBarrier.oldLayout = oldImageLayout;
  imageMemoryBarrier.newLayout = newImageLayout;
  imageMemoryBarrier.image = image;
  imageMemoryBarrier.subresourceRange = subresourceRange;

  // Source layouts (old)
  // Source access mask controls actions that have to be finished on the old layout
  // before it will be transitioned to the new layout
  switch (oldImageLayout)
  {
  case VK_IMAGE_LAYOUT_UNDEFINED:
    // Image layout is undefined (or does not matter)
    // Only valid as initial layout
    // No flags required, listed only for completeness
    imageMemoryBarrier.srcAccessMask = 0;
    break;

  case VK_IMAGE_LAYOUT_PREINITIALIZED:
    // Image is preinitialized
    // Only valid as initial layout for linear images, preserves memory contents
    // Make sure host writes have been finished
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    // Image is a color attachment
    // Make sure any writes to the color buffer have been finished
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    // Image is a depth/stencil attachment
    // Make sure any writes to the depth/stencil buffer have been finished
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    // Image is a transfer source
    // Make sure any reads from the image have been finished
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    break;

  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    // Image is a transfer destination
    // Make sure any writes to the image have been finished
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    // Image is read by a shader
    // Make sure any shader reads from the image have been finished
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    break;
  default:
    // Other source layouts aren't handled (yet)
    break;
  }

  // Target layouts (new)
  // Destination access mask controls the dependency for the new image layout
  switch (newImageLayout)
  {
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    // Image will be used as a transfer destination
    // Make sure any writes to the image have been finished
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    // Image will be used as a transfer source
    // Make sure any reads from the image have been finished
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    break;

  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    // Image will be used as a color attachment
    // Make sure any writes to the color buffer have been finished
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    // Image layout will be used as a depth/stencil attachment
    // Make sure any writes to depth/stencil buffer have been finished
    imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    break;

  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    // Image will be read in a shader (sampler, input attachment)
    // Make sure any writes to the image have been finished
    if (imageMemoryBarrier.srcAccessMask == 0)
    {
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    break;
  default:
    // Other source layouts aren't handled (yet)
    break;
  }

  // Put barrier inside setup command buffer
  vkCmdPipelineBarrier(
    cmdbuffer,
    srcStageMask,
    dstStageMask,
    0,
    0, nullptr,
    0, nullptr,
    1, &imageMemoryBarrier);
}

//Convert this coordinate system:
// (0, 0)           (1280, 0)
//  
//        (640,360)         
//
// (0, 720)       (1280, 720)
// To this:
// (-1,-1)   -1   (1, -1)
//  
// -1         0         1
//
// (-1, 1)    1    (1, 1)
inline void convertPixelToNormalizedDeviceCoord(VkExtent2D swapChainExtent, int x, int y) {
  // float x_norm = 2.0f * x / swapChainExtent.width - 1.0f;
  // float y_norm = 2.0f * y / swapChainExtent.height - 1.0f;

  // std::cout << "NormX: " << x_norm << " NormY:" << y_norm << "\n";

  //First get the min and max coord X and Y of Pixel vs Device
  float xPixelMin = 0;
  float xPixelMax = static_cast<float>(swapChainExtent.width);
  float yPixelMin = 0;
  float yPixelMax = static_cast<float>(swapChainExtent.height);

  float xDeviceMin = -1;
  float xDeviceMax = 1;
  float yDeviceMin = -1;
  float yDeviceMax = 1;

  // Can first normalize the pixel coord space to 0, 1, by normalization by division of value by total size of coordinate
  // e.g. x / (xmax - xmin)
  // Then convert to new coordinate system by multiplying by new max
  // e.g. newX * (newxmax - newxmin)
  // Then translate by the minimum of new coordinate system
  // e.g. finalX = newX + newxmin

  // Or just calcuate ratios between pixel coord vs device coord
  float xScalingFactor = (xDeviceMax - xDeviceMin) / (xPixelMax - xPixelMin);
  float yScalingFactor = (yDeviceMax - yDeviceMin) / (yPixelMax - yPixelMin);

  //Add the dest min to translate to device coord
  float x_norm2 = x * xScalingFactor + xDeviceMin;
  float y_norm2 = y * yScalingFactor + yDeviceMin;

  std::cout << "NormX: " << x_norm2 << " NormY:" << y_norm2 << "\n";


}

inline float convertCoordinate(float srcMin, float srcMax, float dstMin, float dstMax, float srcVal) {

  // Can first normalize the pixel coord space to 0, 1, by normalization by division of value by total size of coordinate
  // e.g. x / (xmax - xmin)
  // Then convert to new coordinate system by multiplying by new max
  // e.g. newX * (newxmax - newxmin)
  // Then translate by the minimum of new coordinate system
  // e.g. finalX = newX + newxmin

  // Or just calcuate ratios between pixel coord vs device coord
  float xScalingFactor = (dstMax - dstMin) / (srcMax - srcMin);

  //Add the dest min to translate to device coord
  float dstVal = srcVal * xScalingFactor + dstMin;

  return dstVal;


}

inline float calculateTriangleFaceDirection(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) {
  /*
  https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFrontFace.html
(-1/2)  ( ((p0.x)(p1.y) - (p1.x)(p0.y)) + ((p1.x)(p2.y) - (p2.x)(p1.y)) + ((p2.x)(p0.y) - (p0.x)(p2.y)) )
(-1/2)  ( ((-0.5)(-0.5) - (0.5)(-0.5)) + ((0.5)(0.5) - (0.5)(-0.5)) + ((0.5)(-0.5) - (-0.5)(0.5)) )
  */
  float sum1 = (p0.x * p1.y) - (p1.x * p0.y);
  float sum2 = (p1.x * p2.y) - (p2.x * p1.y);
  float sum3 = (p2.x * p0.y) - (p0.x * p2.y);
  
  std::cout << "sum1:" << sum1 << " sum2:" << sum2 << " sum3:" << sum3 << "\n";
  float resval = (-0.5f) * (sum1 + sum2 + sum3);
  std::cout << "TriVal: " << resval << "\n";

  // VK_FRONT_FACE_COUNTER_CLOCKWISE specifies that a triangle with positive area is considered front-facing.
  // VK_FRONT_FACE_CLOCKWISE specifies that a triangle with negative area is considered front-facing.

  return resval;

}

} // namespace VulkanHelper
