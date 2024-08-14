#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

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

  glm::vec3 mPosition;
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

inline void updateRotation(float pitch, float yaw){
  float cosYaw = cos(glm::radians(yaw));
  float sinYaw = sin(glm::radians(yaw));

  float cosPitch = cos(glm::radians(pitch));
  float sinPitch = sin(glm::radians(pitch));

  glm::vec3 rotation = glm::vec3(0.0f);

  rotation.x = cosYaw * cosPitch;
  rotation.y = sinPitch;
  rotation.z = sinYaw * cosPitch;

  rotation = glm::normalize(rotation);
  //mVulkanRenderer->mCameraFront = rotation;

}

inline glm::mat4 calculateViewMatrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp){
// 1. Position = known
    // 2. Calculate cameraDirection
    glm::vec3 zaxis = glm::normalize(position - target);
    // 3. Get positive right axis vector
    glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(worldUp), zaxis));
    // 4. Calculate camera up vector
    glm::vec3 yaxis = glm::cross(zaxis, xaxis);

    // Create translation and rotation matrix
    // In glm we access elements as mat[col][row] due to column-major layout
    glm::mat4 translation = glm::mat4(1.0f); // Identity matrix by default
    translation[3][0] = -position.x; // Third column, first row
    translation[3][1] = -position.y;
    translation[3][2] = -position.z;
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0][0] = xaxis.x; // First column, first row
    rotation[1][0] = xaxis.y;
    rotation[2][0] = xaxis.z;
    rotation[0][1] = yaxis.x; // First column, second row
    rotation[1][1] = yaxis.y;
    rotation[2][1] = yaxis.z;
    rotation[0][2] = zaxis.x; // First column, third row
    rotation[1][2] = zaxis.y;
    rotation[2][2] = zaxis.z; 

    // Return lookAt matrix as combination of translation and rotation matrix
    return rotation * translation; // Remember to read from right to left (first translation then rotation)
}

inline glm::mat4 calculateViewMatrixQuat(glm::vec3 position, float pitch, float yaw) {
  //FPS camera:  RotationX(pitch) * RotationY(yaw)
  glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
  glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0));
  //glm::quat qRoll = glm::angleAxis(mRoll ,glm::vec3(0,0,1));  

  //For a FPS camera we can omit roll
  glm::quat orientation = qPitch * qYaw;
  orientation = glm::normalize(orientation);
  //glm::mat4 rotate = glm::mat4_cast(orientation);
  glm::mat cameraRotation = glm::mat4_cast(orientation);

  glm::mat4 translation = glm::mat4(1.0f); // Identity matrix by default
  translation[3][0] = -position.x; // Third column, first row
  translation[3][1] = -position.y;
  translation[3][2] = -position.z;

  return cameraRotation * translation;
}
} // namespace Utils
