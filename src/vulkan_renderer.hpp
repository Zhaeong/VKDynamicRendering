#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include <vulkan_helper.hpp>
#include <vulkan_initializers.hpp>

#include <text_overlay.hpp>

#include <filesystem>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

namespace VulkanEngine {

class VulkanRenderer {
public:
  SDL_Window *mWindow;

  // Device Specific===================================
  std::vector<const char *> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};

  VkInstance mInstance;

  // This needs to be false else other layers don't work
  const bool mEnableValidationLayers = false;

  VkSurfaceKHR mSurface;

  std::vector<const char *> mDeviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
      VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME,
      VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME};

  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;

  VkDevice mLogicalDevice;

  Utils::QueueFamilyIndices mQueueFamilyIndices;

  VkQueue mGraphicsQueue;
  VkQueue mPresentQueue;

  //===================================================
  // Command Submission
  uint32_t mCurrentSwapChainImage = 0;

  VkCommandPool mCommandPool;
  std::vector<VkCommandBuffer> mDrawingCommandBuffers;

  std::vector<VkSemaphore> mImageAvailableSemaphores;
  std::vector<VkSemaphore> mRenderFinishedSemaphores;
  std::vector<VkFence> mInFlightFences;

  //===================================================
  // Swapchain
  VkSwapchainKHR mSwapChain;
  uint32_t mSwapChainImageCount;
  std::vector<VkImage> mSwapChainImages;
  VkFormat mSwapChainImageFormat;
  VkExtent2D mSwapChainExtent;
  std::vector<VkImageView> mSwapChainImageViews;

  //===================================================
  // Pipeline
  VkPipelineLayout mPipelineLayout;
  VkPipeline mGraphicsPipeline;
  //===================================================
  // Pipeline inputs

  // This is set first as VK_NULL_HANDLE, since we initially want to destroy
  // buffer, and VK_NULL_HANDLE is valid for vkDestroyBuffer when buffer in
  // unitialized

  std::vector<Utils::Vertex> mVertices;
  std::vector<uint16_t> mIndices;
  VkBuffer mVertexBuffer = VK_NULL_HANDLE;
  VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;

  VkBuffer mIndexBuffer = VK_NULL_HANDLE;
  VkDeviceMemory mIndexBufferMemory = VK_NULL_HANDLE;

  std::vector<VkBuffer> mUniformBuffers;
  std::vector<VkDeviceMemory> mUniformBuffersMemory;

  std::vector<Utils::Texture> mTextures;

  VkDescriptorPool mDescriptorPool;
  VkDescriptorSetLayout mDescriptorSetLayout{VK_NULL_HANDLE};
  std::vector<VkDescriptorSet> mDescriptorSets;


  TextOverlay *mTextOverlay;

  //===================================================
  // Functions
  VulkanRenderer(SDL_Window *sdlWindow);
  ~VulkanRenderer();

  // Initial object creation
  void createInstance();
  void pickPhysicalDevice();
  void createLogicalDevice();

  // Command Submission objects
  void createCommandPool();
  void createCommandBuffers(uint32_t number);
  void createSyncObjects(uint32_t number);

  void buildDrawingCommandBuffers();

  // Swapchain
  void createSwapChain(VkSurfaceKHR surface);
  void createSwapChainImageViews();

  //DescriptorSetLayout, pipelineLayout
  void setupDescriptorSetLayout();
  // Pipeline
  void createGraphicsPipeline();

  // Pipeline inputs
  void createVertexBuffer(std::vector<Utils::Vertex> vertices);
  void createIndexBuffer(std::vector<uint16_t> indices);
  void createUniformBuffers(int number);

  void loadTextures();
  
  void createDescriptorPool(int number);
  void createDescriptorSets();

  // Rendering functionality

  void updateUniformBuffer(uint32_t currentImage);

  void drawFromVertices(VkCommandBuffer commandBuffer,
                        VkPipeline graphicsPipeline,
                        std::vector<Utils::Vertex> vertices,
                        VkBuffer vertexBuffer);

  void drawFromIndices(VkCommandBuffer commandBuffer,
                       VkPipeline graphicsPipeline,
                       std::vector<Utils::Vertex> vertices,
                       std::vector<uint16_t> indices, VkBuffer vertexBuffer,
                       VkBuffer indexBuffer);

  void drawFromDescriptors(VkCommandBuffer commandBuffer,
                           VkPipeline graphicsPipeline,
                           std::vector<Utils::Vertex> vertices,
                           std::vector<uint16_t> indices,
                           VkBuffer vertexBuffer,
                           VkBuffer indexBuffer);

  void drawFrame();
};
} // namespace VulkanEngine