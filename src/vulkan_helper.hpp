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

} // namespace VulkanHelper
