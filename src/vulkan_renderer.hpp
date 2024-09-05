#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include <vulkan_helper.hpp>
#include <vulkan_initializers.hpp>

#include <text_overlay.hpp>

#include <filesystem>
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
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
      VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
      VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
      VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME};

  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;

  VkDevice mLogicalDevice;

  Utils::QueueFamilyIndices mQueueFamilyIndices;

  VkQueue mGraphicsQueue;
  VkQueue mPresentQueue;

  VkSampleCountFlagBits mMsaaSamples;
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

  //color image for msaa
  VkImage mColorImage;
  VkDeviceMemory mColorImageMemory;
  VkImageView mColorImageView;
  //===================================================
  // Pipeline
  VkPipelineLayout mPipelineLayout;
  VkPipeline mGraphicsPipeline;
  //===================================================
  // Pipeline inputs

  VkBuffer mUBOScene;
  VkDeviceMemory mUBOSceneMemory;

  //VkBuffer mUBOModel;
  //VkDeviceMemory mUBOModelMemory;

  std::vector<Utils::Texture> mTextures;

  VkDescriptorPool mDescriptorPool;
  VkDescriptorSetLayout mDescriptorSetLayout{VK_NULL_HANDLE};
  std::vector<VkDescriptorSet> mDescriptorSets;


  TextOverlay *mTextOverlay;

  
  //===================================================
  //Models abstraction
  std::vector<Utils::Model> mModels;

  //===================================================
  glm::vec3 mCameraPos;
  glm::mat4 mCameraRotation;

  glm::mat4 mViewMatrix;
  //===================================================
  // Functions
  VulkanRenderer(SDL_Window *sdlWindow);
  ~VulkanRenderer();

  // Initial object creation
  void beginVulkanObjectCreation();
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
  void createColorResources();

  //DescriptorSetLayout, pipelineLayout
  void setupDescriptorSetLayout();
  // Pipeline
  void createGraphicsPipeline();

  // Pipeline inputs
  //void createVertexBuffer(std::vector<Utils::Vertex> vertices);
  void createVertexBuffer(std::vector<Utils::Vertex> vertices, VkBuffer *vertexBuffer, VkDeviceMemory *vertexBufferMemory);
  //void createIndexBuffer(std::vector<uint32_t> indices);
  void createIndexBuffer(std::vector<uint32_t> indices, VkBuffer *indexBuffer, VkDeviceMemory *indexBufferMemory); 
  void createUniformBuffers();

  void createUniformBufferForModel(VkBuffer *modelBuffer, VkDeviceMemory *modelMemory);
  void loadTextures();
  
  void createDescriptorPool(int number);
  void createDescriptorSets();
  void createDescriptorSetsModel();

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
                           std::vector<uint32_t> indices,
                           VkBuffer vertexBuffer,
                           VkBuffer indexBuffer,
                           VkDescriptorSet descriptorSet);

  void drawFrame();
};
} // namespace VulkanEngine
