#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include <vulkan_initializers.hpp>

namespace VulkanEngine {

class VulkanRenderer {
public:
  SDL_Window *mWindow;

  std::vector<const char *> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};

  VkInstance mInstance;

  // This needs to be false else other layers don't work
  const bool mEnableValidationLayers = true;

  VkSurfaceKHR mSurface;

  std::vector<const char *> mDeviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME,
      VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME};

  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;

  VkDevice mLogicalDevice;

  VkQueue mGraphicsQueue;
  VkQueue mPresentQueue;

  //===================================================
  // Functions
  VulkanRenderer(SDL_Window *sdlWindow);
  ~VulkanRenderer();

  // Initial object creation
  void createInstance();
  void pickPhysicalDevice();
  void createLogicalDevice();
};
} // namespace VulkanEngine