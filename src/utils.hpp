#pragma once

#include <vulkan/vulkan.h>
#include <vulkan_initializers.hpp>

namespace Utils {
struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  uint32_t presentFamily;
};
} // namespace Utils