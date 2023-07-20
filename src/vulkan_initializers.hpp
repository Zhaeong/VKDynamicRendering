#pragma once
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <utils.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace VulkanInit {

//===================================================
// Vulkan Create struct helpers
//===================================================
inline VkApplicationInfo application_info() {
  VkApplicationInfo application_info{};
  application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  application_info.pApplicationName = "SDLVulkanWithDyn";
  application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  application_info.pEngineName = "No Engine";
  application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  // application_info.apiVersion = VK_API_VERSION_1_2;
  application_info.apiVersion = VK_API_VERSION_1_1;
  return application_info;
}

inline VkInstanceCreateInfo
instance_create_info(VkApplicationInfo appInfo, uint32_t enabledExtensionCount,
                     const char *const *enabledExtensionNames,
                     uint32_t enabledLayerCount,
                     const char *const *enabledLayerNames) {
  VkInstanceCreateInfo instance_create_info{};

  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo = &appInfo;
  instance_create_info.enabledExtensionCount = enabledExtensionCount;
  if (enabledExtensionCount > 0) {
    instance_create_info.ppEnabledExtensionNames = enabledExtensionNames;
  }
  instance_create_info.enabledLayerCount = enabledLayerCount;
  if (enabledLayerCount > 0) {

    instance_create_info.ppEnabledLayerNames = enabledLayerNames;
  }

  instance_create_info.pNext = nullptr;

  return instance_create_info;
}

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

  return indices;
}
} // namespace VulkanInit