#pragma once
#include <SDL2/SDL_vulkan.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <set>
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
  application_info.apiVersion = VK_API_VERSION_1_3;
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

inline VkDeviceQueueCreateInfo
device_queue_create_info(uint32_t queueFamilyIndex, uint32_t queueCount,
                         const float *queuePriority) {
  VkDeviceQueueCreateInfo device_queue_create_info{};
  device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  device_queue_create_info.queueFamilyIndex = queueFamilyIndex;
  device_queue_create_info.queueCount = queueCount;
  device_queue_create_info.pQueuePriorities = queuePriority;
  return device_queue_create_info;
}

inline VkDeviceCreateInfo device_create_info(
    uint32_t queueCreateInfoCount, VkDeviceQueueCreateInfo *queueCreateInfo,
    uint32_t enabledLayerCount, const char *const *enabledLayerNames,
    uint32_t enabledExtensionCount, const char *const *enabledExtensionNames,
    VkPhysicalDeviceFeatures *enabledFeatures) {
  VkDeviceCreateInfo device_create_info{};

  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  device_create_info.queueCreateInfoCount = queueCreateInfoCount;
  device_create_info.pQueueCreateInfos = queueCreateInfo;

  device_create_info.enabledLayerCount = enabledLayerCount;
  device_create_info.ppEnabledLayerNames = enabledLayerNames;

  device_create_info.enabledExtensionCount = enabledExtensionCount;
  device_create_info.ppEnabledExtensionNames = enabledExtensionNames;

  device_create_info.pEnabledFeatures = enabledFeatures;

  return device_create_info;
}

inline VkCommandPoolCreateInfo command_pool_create_info() {
  VkCommandPoolCreateInfo command_pool_create_info{};
  command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  return command_pool_create_info;
}

inline VkCommandBufferAllocateInfo
command_buffer_allocate_info(VkCommandPool command_pool,
                             VkCommandBufferLevel level,
                             uint32_t buffer_count) {
  VkCommandBufferAllocateInfo command_buffer_allocate_info{};
  command_buffer_allocate_info.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_allocate_info.commandPool = command_pool;
  command_buffer_allocate_info.level = level;
  command_buffer_allocate_info.commandBufferCount = buffer_count;
  return command_buffer_allocate_info;
}

} // namespace VulkanInit