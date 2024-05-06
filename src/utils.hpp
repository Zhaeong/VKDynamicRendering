#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>


/// @brief Helper macro to test the result of Vulkan calls which can return an
/// error.
/// Check err codes https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkResult.html
#define VK_CHECK(x, s)                                                         \
  do {                                                                         \
    VkResult err = x;                                                          \
    if (err) {                                                                 \
      std::cout << "Detected Vulkan error: " << s << " errnum:" << err << "\n";                     \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#define VK_FLAGS_NONE 0        // Custom define for better code readability

#define DEFAULT_FENCE_TIMEOUT 100000000000        // Default fence timeout in nanoseconds

namespace Utils {
struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  uint32_t presentFamily;
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }

  static std::vector<VkVertexInputAttributeDescription>
  getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    attributeDescriptions.resize(4);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, color);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
  }
};

struct UniformBufferObject {
  glm::mat4 view;
  glm::mat4 proj;
  glm::vec4 light;
  glm::vec3 camPos;
};

struct UniformBufferObjectModel {
  glm::mat4 modelPos;
};

struct Texture {
  std::string texPath;
  VkSampler sampler;
  VkImage image;
  VkImageLayout image_layout;
  VkDeviceMemory device_memory;
  VkImageView view;
  uint32_t width, height;
  uint32_t mip_levels;
  uint32_t descriptor_set_index;
};

struct Model{
  std::vector<Utils::Vertex> mVertices;
  std::vector<uint32_t> mIndices;

  // This is set first as VK_NULL_HANDLE, since we initially want to destroy
  // buffer, and VK_NULL_HANDLE is valid for vkDestroyBuffer when buffer in
  // unitialized

  VkBuffer mVertexBuffer = VK_NULL_HANDLE;
  VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;

  VkBuffer mIndexBuffer = VK_NULL_HANDLE;
  VkDeviceMemory mIndexBufferMemory = VK_NULL_HANDLE;

  VkBuffer mUniformBuffer = VK_NULL_HANDLE;
  VkDeviceMemory mUniformBuffersMemory = VK_NULL_HANDLE;

  VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
};

inline void showWindowFlags(int flags) {

  printf("\nFLAGS ENABLED: ( %d )\n", flags);
  printf("=======================\n");
  if (flags & SDL_WINDOW_FULLSCREEN)
    printf("SDL_WINDOW_FULLSCREEN\n");
  if (flags & SDL_WINDOW_OPENGL)
    printf("SDL_WINDOW_OPENGL\n");
  if (flags & SDL_WINDOW_SHOWN)
    printf("SDL_WINDOW_SHOWN\n");
  if (flags & SDL_WINDOW_HIDDEN)
    printf("SDL_WINDOW_HIDDEN\n");
  if (flags & SDL_WINDOW_BORDERLESS)
    printf("SDL_WINDOW_BORDERLESS\n");
  if (flags & SDL_WINDOW_RESIZABLE)
    printf("SDL_WINDOW_RESIZABLE\n");
  if (flags & SDL_WINDOW_MINIMIZED)
    printf("SDL_WINDOW_MINIMIZED\n");
  if (flags & SDL_WINDOW_MAXIMIZED)
    printf("SDL_WINDOW_MAXIMIZED\n");
  if (flags & SDL_WINDOW_INPUT_GRABBED)
    printf("SDL_WINDOW_INPUT_GRABBED\n");
  if (flags & SDL_WINDOW_INPUT_FOCUS)
    printf("SDL_WINDOW_INPUT_FOCUS\n");
  if (flags & SDL_WINDOW_MOUSE_FOCUS)
    printf("SDL_WINDOW_MOUSE_FOCUS\n");
  if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
    printf("SDL_WINDOW_FULLSCREEN_DESKTOP\n");
  if (flags & SDL_WINDOW_FOREIGN)
    printf("SDL_WINDOW_FOREIGN\n");
  printf("=======================\n");
}

} // namespace Utils
