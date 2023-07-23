#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include <vulkan_initializers.hpp>

namespace VulkanEngine {

class VulkanRenderer {
public:
  SDL_Window *mWindow;

  // Device Specific===================================
  std::vector<const char *> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};

  VkInstance mInstance;

  // This needs to be false else other layers don't work
  const bool mEnableValidationLayers = true;

  VkSurfaceKHR mSurface;

  std::vector<const char *> mDeviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
      VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME,
      VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME};

  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;

  VkDevice mLogicalDevice;

  VkQueue mGraphicsQueue;
  VkQueue mPresentQueue;

  // Command Submission================================
  const int MAX_FRAMES_IN_FLIGHT = 2;

  VkCommandPool mCommandPool;
  std::vector<VkCommandBuffer> mCommandBuffers;

  std::vector<VkSemaphore> mImageAvailableSemaphores;
  std::vector<VkSemaphore> mRenderFinishedSemaphores;
  std::vector<VkFence> mInFlightFences;

  //===================================================

  //===================================================
  // Functions
  VulkanRenderer(SDL_Window *sdlWindow);
  ~VulkanRenderer();

  // Initial object creation
  void createInstance();
  void pickPhysicalDevice();
  void createLogicalDevice();

  // Command objects
  void createCommandPool();
  void createCommandBuffers(uint32_t number);
  void createSyncObjects(uint32_t number);
};
} // namespace VulkanEngine