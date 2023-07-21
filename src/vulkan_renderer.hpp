#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include <vulkan_initializers.hpp>

namespace VulkanEngine {

class VulkanRenderer {
public:
  SDL_Window *mWindow;

  VkInstance mInstance;

  // This needs to be false else other layers don't work
  const bool mEnableValidationLayers = true;

  VkSurfaceKHR mSurface;

  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;

  //===================================================
  // Functions
  VulkanRenderer(SDL_Window *sdlWindow);
  ~VulkanRenderer();

  // Initial object creation
  void createInstance();
  void pickPhysicalDevice();
};
} // namespace VulkanEngine