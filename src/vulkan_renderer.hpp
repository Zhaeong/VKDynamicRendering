#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include <vulkan_helper.hpp>
#include <vulkan_initializers.hpp>

#include <text_overlay.hpp>

#include <filesystem>
#include <string>

#define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

namespace VulkanEngine {

class VulkanRenderer {
public:
  SDL_Window *mWindow;

  // Device Specific===================================
  std::vector<const char *> mValidationLayers = {};

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

  //Depth image
  VkImage mDepthImage;
  VkDeviceMemory mDepthImageMemory;
  VkImageView mDepthImageView;

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
  float mCameraPosX = 0.0f;
  float mCameraPosY = 0.0f;
  float mCameraPosZ = 2.0f;

  float mCameraLookX = 0.0f;
  float mCameraLookY = 0.0f;
  float mCameraLookZ = 0.0f;


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
  void cleanupSwapChain();
  void recreateSwapChain();

  void createDepthImage();

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
