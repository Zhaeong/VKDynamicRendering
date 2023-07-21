#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace Utils {
struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  uint32_t presentFamily;
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

} // namespace Utils