#include <vulkan_renderer.hpp>

namespace VulkanEngine {

VulkanRenderer::VulkanRenderer(SDL_Window *sdlWindow) {
  mWindow = sdlWindow;
  createInstance();

  if (SDL_Vulkan_CreateSurface(mWindow, mInstance, &mSurface) != SDL_TRUE) {
    throw std::runtime_error("failed to create window surface!");
  }

  pickPhysicalDevice();
  createLogicalDevice();

  // createCommandPool();
  // createCommandBuffers(MAX_FRAMES_IN_FLIGHT);
  // createSyncObjects(MAX_FRAMES_IN_FLIGHT);

  createSwapChain(mSurface);
  createSwapChainImageViews();


  mTextOverlay = new TextOverlay(mPhysicalDevice, mLogicalDevice, mQueueFamilyIndices.graphicsFamily, mSwapChainImageViews, mSwapChainImageFormat, mSwapChainExtent, mGraphicsQueue);


  mTextOverlay->beginTextUpdate();
  mTextOverlay->addText("aaaabbb", 5.0f, 5.0f, TextOverlay::alignLeft);
  mTextOverlay->endTextUpdate();

  createCommandPool();
  createCommandBuffers(mSwapChainImageCount);
  createSyncObjects(mSwapChainImageCount);

  //loaded before creating the descriptor set bindings
  loadTextures();

  //Required for pipeline layout before creation of graphics pipeline
  setupDescriptorSetLayout();

  createGraphicsPipeline();

  // Creation of custom input objects into the graphics pipeline
  // std::vector<Utils::Vertex> vertices = {
  //     {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
  //     {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
  //     {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
  //     {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},

  //     {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
  //     {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
  //     {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
  //     {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}};

  //Direction of coordinates
  // (-1,-1)   -1   (1, -1)
  //  
  // -1         0         1
  //
  // (-1, 1)    1    (1, 1)

  // std::vector<Utils::Vertex> vertices = {{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, //top left
  //                                        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  //top right
  //                                        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},    //bottom right
  //                                        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}};  //bottom left

  std::vector<Utils::Vertex> vertices = {
                                         {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, 
                                         {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  
                                         {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},  
                                         {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

                                         {{-0.5f, -0.5f, 0.2f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, 
                                         {{0.5f, -0.5f, 0.2f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  
                                         {{0.5f, 0.5f, 0.2f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},  
                                         {{-0.5f, 0.5f, 0.2f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
                                         };  //bottom left

                                         

  createVertexBuffer(vertices);
  //rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  mIndices = {0, 1, 2, 2, 3, 0,
              4, 5, 6, 6, 7, 4};
  // mIndices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

  createIndexBuffer(mIndices);

  createUniformBuffers(1);
  createDescriptorPool(1);
  createDescriptorSets();

  buildDrawingCommandBuffers();
}

VulkanRenderer::~VulkanRenderer() {
  vkDestroyBuffer(mLogicalDevice, mVertexBuffer, nullptr);
  vkFreeMemory(mLogicalDevice, mVertexBufferMemory, nullptr);

  vkDestroyBuffer(mLogicalDevice, mIndexBuffer, nullptr);
  vkFreeMemory(mLogicalDevice, mIndexBufferMemory, nullptr);

  for (size_t i = 0; i < mUniformBuffers.size(); i++) {
    vkDestroyBuffer(mLogicalDevice, mUniformBuffers[i], nullptr);
    vkFreeMemory(mLogicalDevice, mUniformBuffersMemory[i], nullptr);
  }

  for (auto imageView : mSwapChainImageViews) {
    vkDestroyImageView(mLogicalDevice, imageView, nullptr);
  }
  vkDestroySwapchainKHR(mLogicalDevice, mSwapChain, nullptr);

  for (size_t i = 0; i < mImageAvailableSemaphores.size(); i++) {
    vkDestroySemaphore(mLogicalDevice, mRenderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(mLogicalDevice, mImageAvailableSemaphores[i], nullptr);
    vkDestroyFence(mLogicalDevice, mInFlightFences[i], nullptr);
  }

  vkDestroyCommandPool(mLogicalDevice, mCommandPool, nullptr);
  vkDestroyDevice(mLogicalDevice, nullptr);
  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vkDestroyInstance(mInstance, nullptr);
}

void VulkanRenderer::createInstance() {

  if (mEnableValidationLayers &&
      !VulkanHelper::iCheckValidationLayerSupport(mValidationLayers)) {
    throw std::runtime_error("Validation Layer requested but not available\n");
  } else if (mEnableValidationLayers) {
    std::cout << "Enabling Validation Layers\n";
  } else {
    std::cout << "ValidationLayers Not enabled\n";
  }

  std::vector<const char *> requiredVkExtenstionsForSDL =
      VulkanHelper::iGetRequiredVkExtensions(mWindow);

  VkApplicationInfo appInfo = VulkanInit::application_info();

  uint32_t enabledLayerCount = 0;
  const char *const *enabledLayerNames;

  if (mEnableValidationLayers) {
    enabledLayerCount = mValidationLayers.size();
    enabledLayerNames = mValidationLayers.data();
  }

  VkInstanceCreateInfo instance_create_info{};
  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo = &appInfo;
  instance_create_info.enabledExtensionCount = requiredVkExtenstionsForSDL.size();
  if (requiredVkExtenstionsForSDL.size() > 0) {
    instance_create_info.ppEnabledExtensionNames = requiredVkExtenstionsForSDL.data();
  }
  instance_create_info.enabledLayerCount = enabledLayerCount;
  if (enabledLayerCount > 0) {

    instance_create_info.ppEnabledLayerNames = enabledLayerNames;
  }

  instance_create_info.pNext = nullptr;

  VK_CHECK(vkCreateInstance(&instance_create_info, nullptr, &mInstance),
           "vkCreateInstance");
}

void VulkanRenderer::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  } else {
    std::cout << "Num Physical Devices: " << deviceCount << "\n";
  }

  std::vector<VkPhysicalDevice> vPhysicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(mInstance, &deviceCount, vPhysicalDevices.data());

  for (int i = 0; i < vPhysicalDevices.size(); i++) {
    std::cout << "Device Index: " << i << "\n";
    if (VulkanHelper::iIsDeviceSuitable(vPhysicalDevices[i], mSurface,
                                        mDeviceExtensions)) {
      mPhysicalDevice = vPhysicalDevices[i];
      break;
    }
  }

  if (mPhysicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error(
        "Failed to find Physical Device with all required features");
  } else {

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &deviceProperties);
    std::cout << "Picked " << deviceProperties.deviceName
              << " Vendor: " << deviceProperties.vendorID << "\n";

    std::cout << "Max bound descriptorSets: " <<  deviceProperties.limits.maxBoundDescriptorSets << "\n";
  }
}

void VulkanRenderer::createLogicalDevice() {

  // Specifying queues to be created
  mQueueFamilyIndices = VulkanHelper::iFindQueueFamilies(mPhysicalDevice, mSurface);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

  // set will only contain unique values
  std::set<uint32_t> uniqueQueueFamilies = {mQueueFamilyIndices.graphicsFamily,
                                            mQueueFamilyIndices.presentFamily};

  // create device queue
  // Assigns priorty to queues to influence scheduling of comand buffer
  // execution
  float queuePriority = 1.0f;
  for (int i = 0; i < uniqueQueueFamilies.size(); i++) {
    VkDeviceQueueCreateInfo queueCreateInfo =
        VulkanInit::device_queue_create_info(i, 1, &queuePriority);
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  // Specifying used device features
  VkPhysicalDeviceFeatures deviceFeatures{};
  // enable anisotropy
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  uint32_t enabledLayerCount = 0;
  const char *const *enabledLayerNames;

  if (mEnableValidationLayers) {
    enabledLayerCount = mValidationLayers.size();
    enabledLayerNames = mValidationLayers.data();
  }

  VkDeviceCreateInfo createInfo = VulkanInit::device_create_info(
      queueCreateInfos.size(), queueCreateInfos.data(), enabledLayerCount,
      enabledLayerNames, mDeviceExtensions.size(), mDeviceExtensions.data(),
      &deviceFeatures);

  // Query vk12 features
  // VkPhysicalDeviceVulkan12Features vk12Features{};
  // vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  // createInfo.pNext = &vk12Features;
  VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature{};
  dynamic_rendering_feature.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
  dynamic_rendering_feature.dynamicRendering = VK_TRUE;

  createInfo.pNext = &dynamic_rendering_feature;

  if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicalDevice) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(mLogicalDevice, mQueueFamilyIndices.graphicsFamily, 0, &mGraphicsQueue);

  // Now create the present queue
  vkGetDeviceQueue(mLogicalDevice, mQueueFamilyIndices.presentFamily, 0, &mPresentQueue);
}

void VulkanRenderer::createCommandPool() {

  VkCommandPoolCreateInfo poolInfo = VulkanInit::command_pool_create_info();
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = mQueueFamilyIndices.graphicsFamily;

  VK_CHECK(vkCreateCommandPool(mLogicalDevice, &poolInfo, nullptr, &mCommandPool), "vkCreateCommandPool");
}

void VulkanRenderer::createCommandBuffers(uint32_t number) {

  mDrawingCommandBuffers.resize(number);

  VkCommandBufferAllocateInfo allocInfo =
      VulkanInit::command_buffer_allocate_info(
          mCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          (uint32_t)mDrawingCommandBuffers.size());

  VK_CHECK(vkAllocateCommandBuffers(mLogicalDevice, &allocInfo, mDrawingCommandBuffers.data()),
           "vkAllocateCommandBuffers");
}

void VulkanRenderer::createSyncObjects(uint32_t number) {
  mImageAvailableSemaphores.resize(number);
  mRenderFinishedSemaphores.resize(number);
  mInFlightFences.resize(number);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < number; i++) {
    if (vkCreateSemaphore(mLogicalDevice, &semaphoreInfo, nullptr,
                          &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(mLogicalDevice, &semaphoreInfo, nullptr,
                          &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(mLogicalDevice, &fenceInfo, nullptr,
                      &mInFlightFences[i]) != VK_SUCCESS) {

      throw std::runtime_error(
          "failed to create synchronization objects for a frame!");
    }
  }
}

void VulkanRenderer::buildDrawingCommandBuffers(){
  for (int32_t i = 0; i < mDrawingCommandBuffers.size(); ++i) {
    VulkanHelper::beginDrawingCommandBuffer(mDrawingCommandBuffers[i]);

    VkImageSubresourceRange range{};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = VK_REMAINING_MIP_LEVELS;
    range.baseArrayLayer = 0;
    range.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VulkanInit::insert_image_memory_barrier(
        mDrawingCommandBuffers[i], mSwapChainImages[i],
        0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, range);

    // Normally renderpass, use renderinginfo for dynamic rendering
    VkRenderingAttachmentInfoKHR renderingColorAttachmentInfo{};
    renderingColorAttachmentInfo.sType =
        VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    renderingColorAttachmentInfo.imageView =
        mSwapChainImageViews[i];
    renderingColorAttachmentInfo.imageLayout =
        VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    renderingColorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderingColorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderingColorAttachmentInfo.clearValue = clearColor;

    VkRenderingInfoKHR renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.renderArea.offset = {0, 0};
    renderingInfo.renderArea.extent = mSwapChainExtent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &renderingColorAttachmentInfo;
    // dynamic rendering end
    //===============================================================

    vkCmdBeginRendering(mDrawingCommandBuffers[i], &renderingInfo);

    drawFromDescriptors(mDrawingCommandBuffers[i], mGraphicsPipeline, mVertices,
                    mIndices, mVertexBuffer, mIndexBuffer);

    vkCmdEndRendering(mDrawingCommandBuffers[i]);

    VulkanInit::insert_image_memory_barrier(
        mDrawingCommandBuffers[i], mSwapChainImages[i],
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, range);


    VK_CHECK(vkEndCommandBuffer(mDrawingCommandBuffers[i]), "vkEndCommandBuffer"); 
  }

}

void VulkanRenderer::createSwapChain(VkSurfaceKHR surface) {
  Utils::SwapChainSupportDetails swapChainSupport =
      VulkanHelper::iQuerySwapChainSupport(mPhysicalDevice, surface);

  VkSurfaceFormatKHR surfaceFormat =
      VulkanHelper::iChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode =
      VulkanHelper::iChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent =
      VulkanHelper::iChooseSwapExtent(mWindow, swapChainSupport.capabilities);

  // minimum + 1, and don't go over maximum
  mSwapChainImageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      mSwapChainImageCount > swapChainSupport.capabilities.maxImageCount) {
    mSwapChainImageCount = swapChainSupport.capabilities.maxImageCount;
  }

  std::cout << "SwapChain Image count: " << mSwapChainImageCount << "\n";
  // VK_FORMAT_B8G8R8A8_SRGB = 50,
  std::cout << "SwapChain Image format: " << surfaceFormat.format << "\n";

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.minImageCount = mSwapChainImageCount;
  createInfo.flags = VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR;

  VkImageFormatListCreateInfo formatList{};
  formatList.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
  std::vector formats = {VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB};
  formatList.viewFormatCount = formats.size();
  formatList.pViewFormats = formats.data();

  createInfo.pNext = &formatList;

  createInfo.imageFormat = surfaceFormat.format;

  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndices[] = {mQueueFamilyIndices.graphicsFamily,
                                   mQueueFamilyIndices.presentFamily};

  if (mQueueFamilyIndices.graphicsFamily != mQueueFamilyIndices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;     // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(mLogicalDevice, &createInfo, nullptr, &mSwapChain) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  vkGetSwapchainImagesKHR(mLogicalDevice, mSwapChain, &mSwapChainImageCount,
                          nullptr);
  mSwapChainImages.resize(mSwapChainImageCount);
  vkGetSwapchainImagesKHR(mLogicalDevice, mSwapChain, &mSwapChainImageCount,
                          mSwapChainImages.data());
  mSwapChainImageFormat = surfaceFormat.format;
  mSwapChainExtent = extent;
}

void VulkanRenderer::createSwapChainImageViews() {
  mSwapChainImageViews.resize(mSwapChainImages.size());

  std::cout << "swapchain Image View Format: " << mSwapChainImageFormat << "\n";

  for (size_t i = 0; i < mSwapChainImages.size(); i++) {
    mSwapChainImageViews[i] = VulkanHelper::createImageView(
        mLogicalDevice, mSwapChainImages[i], mSwapChainImageFormat,
        VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

void VulkanRenderer::createGraphicsPipeline() {

  //================================================================================================
  // Create pipeline
  //================================================================================================
  VkGraphicsPipelineCreateInfo PIPElineInfo{};
  PIPElineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

  //================================================================================================
  // Create shader stage
  //================================================================================================
  std::string vertShaderPath = "shaders/simple_shader.vert.spv";
  std::string fragShaderPath = "shaders/simple_shader.frag.spv";
  auto vertShaderCode = VulkanHelper::readFile(vertShaderPath);
  auto fragShaderCode = VulkanHelper::readFile(fragShaderPath);

  // Vertex Shader
  VkShaderModule vertShaderModule =
      VulkanHelper::createShaderModule(mLogicalDevice, vertShaderCode);
  VkShaderModule fragShaderModule =
      VulkanHelper::createShaderModule(mLogicalDevice, fragShaderCode);

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  shaderStages.push_back(VulkanHelper::loadShader(mLogicalDevice, vertShaderCode, VK_SHADER_STAGE_VERTEX_BIT));
  shaderStages.push_back(VulkanHelper::loadShader(mLogicalDevice, fragShaderCode, VK_SHADER_STAGE_FRAGMENT_BIT));

  PIPElineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());;
  PIPElineInfo.pStages = shaderStages.data();
  //================================================================================================
  // pVertexInputState
  //================================================================================================
  auto bindingDescription = Utils::Vertex::getBindingDescription();
  auto attributeDescriptions = Utils::Vertex::getAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional

  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions =
      attributeDescriptions.data(); // Optional

  PIPElineInfo.pVertexInputState = &vertexInputInfo;

  //================================================================================================
  // pInputAssemblyState
  //================================================================================================
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;
  PIPElineInfo.pInputAssemblyState = &inputAssembly;
  //================================================================================================
  // pViewportState
  //================================================================================================
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)mSwapChainExtent.width;
  viewport.height = (float)mSwapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = mSwapChainExtent;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  PIPElineInfo.pViewportState = &viewportState;

  //================================================================================================
  // pRasterizationState
  //================================================================================================
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;

  // Setting this to true disables output to framebuffer
  // rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

  rasterizer.lineWidth = 1.0f;

  rasterizer.cullMode = VK_CULL_MODE_NONE;
  // rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f;          // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

  PIPElineInfo.pRasterizationState = &rasterizer;
  //================================================================================================
  // pMultisampleState
  //================================================================================================
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;          // Optional
  multisampling.pSampleMask = nullptr;            // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE;      // Optional

  PIPElineInfo.pMultisampleState = &multisampling;
  //================================================================================================
  // pDepthStencilState
  //================================================================================================

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f; // Optional
  depthStencil.maxDepthBounds = 1.0f; // Optional

  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front = {}; // Optional
  depthStencil.back = {};  // Optional

  PIPElineInfo.pDepthStencilState = &depthStencil; // Optional
  //================================================================================================
  // pColorBlendState
  //================================================================================================
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  PIPElineInfo.pColorBlendState = &colorBlending;
  //================================================================================================

  PIPElineInfo.pDynamicState = nullptr; // Optional
  //================================================================================================
  // pipelineLayout
  //================================================================================================
  // VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  // pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  // // Descriptor set layouts
  // pipelineLayoutInfo.setLayoutCount = 1; // Optional

  // VkDescriptorSetLayout descriptorSetLayout =
  //     VulkanInit::create_descriptorSetLayout(mLogicalDevice);
  // pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional

  // // Push constants
  // pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
  // pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

  // VK_CHECK(vkCreatePipelineLayout(mLogicalDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout), 
  //          "vkCreatePipelineLayout");

  PIPElineInfo.layout = mPipelineLayout;
  //================================================================================================
  PIPElineInfo.renderPass = nullptr;
  PIPElineInfo.subpass = 0;
  //================================================================================================
  PIPElineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  PIPElineInfo.basePipelineIndex = -1;              // Optional
  //================================================================================================
  // For dynamic rendering
  //================================================================================================
  VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info{};
  pipeline_rendering_create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;

  pipeline_rendering_create_info.colorAttachmentCount = 1;
  pipeline_rendering_create_info.pColorAttachmentFormats =
      &mSwapChainImageFormat;

  pipeline_rendering_create_info.depthAttachmentFormat =
      VulkanHelper::findSupportedFormat(
          mPhysicalDevice,
          {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
           VK_FORMAT_D24_UNORM_S8_UINT},
          VK_IMAGE_TILING_OPTIMAL,
          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  pipeline_rendering_create_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

  PIPElineInfo.pNext = &pipeline_rendering_create_info;

  //================================================================================================
  VK_CHECK(vkCreateGraphicsPipelines(mLogicalDevice, VK_NULL_HANDLE, 1, &PIPElineInfo, nullptr, &mGraphicsPipeline),
           "vkCreateGraphicsPipelines");

  // Cleanup===========
  vkDestroyShaderModule(mLogicalDevice, fragShaderModule, nullptr);
  vkDestroyShaderModule(mLogicalDevice, vertShaderModule, nullptr);
  //================================================================================================
}

void VulkanRenderer::createVertexBuffer(std::vector<Utils::Vertex> vertices) {

  mVertices = vertices;

  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
  // Create a staging buffer as source for cpu accessible then copy over to
  // actual bufffer
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  VulkanHelper::createBuffer(mPhysicalDevice, mLogicalDevice, bufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), (size_t)bufferSize);
  vkUnmapMemory(mLogicalDevice, stagingBufferMemory);

  VulkanHelper::createBuffer(
      mPhysicalDevice, mLogicalDevice, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer, mVertexBufferMemory);

  VulkanHelper::copyBuffer(mLogicalDevice, mCommandPool, mGraphicsQueue,
                           stagingBuffer, mVertexBuffer, bufferSize);
  vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(mLogicalDevice, stagingBufferMemory, nullptr);
}

void VulkanRenderer::createIndexBuffer(std::vector<uint16_t> indices) {
  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  VulkanHelper::createBuffer(mPhysicalDevice, mLogicalDevice, bufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), (size_t)bufferSize);
  vkUnmapMemory(mLogicalDevice, stagingBufferMemory);

  VulkanHelper::createBuffer(
      mPhysicalDevice, mLogicalDevice, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer, mIndexBufferMemory);

  VulkanHelper::copyBuffer(mLogicalDevice, mCommandPool, mGraphicsQueue,
                           stagingBuffer, mIndexBuffer, bufferSize);

  vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(mLogicalDevice, stagingBufferMemory, nullptr);
}

void VulkanRenderer::createUniformBuffers(int number) {
  VkDeviceSize bufferSize = sizeof(Utils::UniformBufferObject);

  mUniformBuffers.resize(number);
  mUniformBuffersMemory.resize(number);

  for (size_t i = 0; i < number; i++) {
    VulkanHelper::createBuffer(mPhysicalDevice, mLogicalDevice, bufferSize,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        mUniformBuffers[i], mUniformBuffersMemory[i]);
  }

}

void VulkanRenderer::loadTextures() {

  std::filesystem::path p = std::filesystem::current_path();

  Utils::Texture firstTexture = VulkanHelper::loadTexture((p.generic_string() + "/textures/amdtexture.jpg").c_str(),
                                                          VK_FORMAT_R8G8B8A8_SRGB,
                                                          mPhysicalDevice,
                                                          mLogicalDevice,
                                                          mCommandPool,
                                                          mGraphicsQueue);
  mTextures.push_back(firstTexture);

  Utils::Texture secondTexture = VulkanHelper::loadTexture((p.generic_string() + "/textures/amdtexture2.jpg").c_str(),
                                                          VK_FORMAT_R8G8B8A8_SRGB,
                                                          mPhysicalDevice,
                                                          mLogicalDevice,
                                                          mCommandPool,
                                                          mGraphicsQueue);
  mTextures.push_back(secondTexture);


}

void VulkanRenderer::createDescriptorPool(int number) {

  std::vector<VkDescriptorPoolSize> poolSizes{};

  VkDescriptorPoolSize poolSizeUBO{};
  poolSizeUBO.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizeUBO.descriptorCount = static_cast<uint32_t>(number);
  poolSizes.push_back(poolSizeUBO);

  VkDescriptorPoolSize poolSizeIMGSampler{};
  poolSizeIMGSampler.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizeIMGSampler.descriptorCount = static_cast<uint32_t>(number);
  poolSizes.push_back(poolSizeIMGSampler);

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(mTextures.size());

  if (vkCreateDescriptorPool(mLogicalDevice, &poolInfo, nullptr, &mDescriptorPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }

}

void VulkanRenderer::setupDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings = {
	    VulkanInit::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
      VulkanInit::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
	};

  // This is if we want to create multiple image samplers
  // for (int i = 0; i < mTextures.size(); i++) {
  //   VkDescriptorSetLayoutBinding imageSamplerBinding = VulkanInit::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, i + 1);
  //   set_layout_bindings.push_back(imageSamplerBinding);
  // }

	VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info =
	    VulkanInit::descriptor_set_layout_create_info(set_layout_bindings.data(), static_cast<uint32_t>(set_layout_bindings.size()));

	VK_CHECK(vkCreateDescriptorSetLayout(mLogicalDevice, &descriptor_layout_create_info, nullptr, &mDescriptorSetLayout), "vkCreateDescriptorSetLayout");

	VkPipelineLayoutCreateInfo pipeline_layout_create_info =
	    VulkanInit::pipeline_layout_create_info(
	        &mDescriptorSetLayout,
	        1);

	VK_CHECK(vkCreatePipelineLayout(mLogicalDevice, &pipeline_layout_create_info, nullptr, &mPipelineLayout), "vkCreatePipelineLayout");
}

void VulkanRenderer::createDescriptorSets()
{
  //Create a descriptor set per image
  std::cout << "Creating: " << mTextures.size() << " DescriptorSets\n";
  mDescriptorSets.resize(mTextures.size());

  //The layouts array must equal to the number of descriptor sets you want to allocate
  std::vector<VkDescriptorSetLayout> layoutsArray;
  for (size_t i = 0; i < mTextures.size(); i++) {
    layoutsArray.push_back(mDescriptorSetLayout);
  }

	VkDescriptorSetAllocateInfo alloc_info =
	    VulkanInit::descriptor_set_allocate_info(
	        mDescriptorPool,
	        layoutsArray.data(),
	        mTextures.size());

	VK_CHECK(vkAllocateDescriptorSets(mLogicalDevice, &alloc_info, mDescriptorSets.data()), "vkAllocateDescriptorSets");

  for (size_t i = 0; i < mTextures.size(); i++) {
    VkDescriptorBufferInfo            matrix_buffer_descriptor     = VulkanInit::create_descriptor_buffer(mUniformBuffers[0], sizeof(Utils::UniformBufferObject), 0);
    VkDescriptorImageInfo environment_image_descriptor = VulkanInit::create_descriptor_texture(mTextures[i]);
    std::vector<VkWriteDescriptorSet> write_descriptor_sets        = {
          VulkanInit::write_descriptor_set_from_buffer(mDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &matrix_buffer_descriptor),
          VulkanInit::write_descriptor_set_from_image(mDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &environment_image_descriptor)
      };

    mTextures[i].descriptor_set_index = i;

    std::cout << "Texture: " << mTextures[i].texPath << " descriptorsetindex: " << mTextures[i].descriptor_set_index << "\n";

    //Allow for dynamic amount of texture samplers to be specified in shader bindings
    // for (int j = 0; j < mTextures.size(); j++) {
    //   VkDescriptorImageInfo environment_image_descriptor = VulkanInit::create_descriptor_texture(mTextures[j]);
    //   VkWriteDescriptorSet descriptorWriteImgSampler = VulkanInit::write_descriptor_set_from_image(mDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, j + 1, &environment_image_descriptor);
    //   write_descriptor_sets.push_back(descriptorWriteImgSampler);
    //   std::cout << "writing texture: " << mTextures[j].texPath << " to descriptor sampler binding: " << j + 1 << "\n";
    // }
    vkUpdateDescriptorSets(mLogicalDevice, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
  }
}

void VulkanRenderer::updateUniformBuffer(uint32_t currentImage) {
  
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(
                   currentTime - startTime)
                   .count();


  Utils::UniformBufferObject ubo{};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                          glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view =
      glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f));

  ubo.proj = glm::perspective(glm::radians(45.0f),
                              mSwapChainExtent.width /
                                  (float)mSwapChainExtent.height,
                              0.1f, 10.0f);

  //ubo.proj[1][1] *= -1;

  void *data;
  vkMapMemory(mLogicalDevice,
              mUniformBuffersMemory[currentImage], 0, sizeof(ubo),
              0, &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(mLogicalDevice,
                mUniformBuffersMemory[currentImage]);
}


void VulkanRenderer::drawFromVertices(VkCommandBuffer commandBuffer,
                                      VkPipeline graphicsPipeline,
                                      std::vector<Utils::Vertex> vertices,
                                      VkBuffer vertexBuffer) {
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    graphicsPipeline);

  VkBuffer vertexBuffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

  std::cout << "Drawing: " << vertices.size() << " Vertices\n";

  vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
}

void VulkanRenderer::drawFromIndices(VkCommandBuffer commandBuffer,
                                     VkPipeline graphicsPipeline,
                                     std::vector<Utils::Vertex> vertices,
                                     std::vector<uint16_t> indices,
                                     VkBuffer vertexBuffer,
                                     VkBuffer indexBuffer) {
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    graphicsPipeline);

  VkBuffer vertexBuffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

  vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0,
                   0, 0);
}


void VulkanRenderer::drawFromDescriptors(VkCommandBuffer commandBuffer,
                                         VkPipeline graphicsPipeline,
                                         std::vector<Utils::Vertex> vertices,
                                         std::vector<uint16_t> indices,
                                         VkBuffer vertexBuffer,
                                         VkBuffer indexBuffer) {

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    graphicsPipeline);

  VkBuffer vertexBuffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0,
                       VK_INDEX_TYPE_UINT16);

  
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mPipelineLayout, 0, 1,
                          &mDescriptorSets[mTextures[0].descriptor_set_index], 0,
                          nullptr);

  //vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
  //first rect
  vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mPipelineLayout, 0, 1,
                          &mDescriptorSets[mTextures[1].descriptor_set_index], 0,
                          nullptr);

  //second rect
  vkCmdDrawIndexed(commandBuffer, 6, 1, 6, 0, 0);

}

void VulkanRenderer::drawFrame() {
  vkWaitForFences(mLogicalDevice, 1, &mInFlightFences[mCurrentSwapChainImage], VK_TRUE,
                  UINT64_MAX);

  VkResult result =
      vkAcquireNextImageKHR(mLogicalDevice, mSwapChain, UINT64_MAX,
                            mImageAvailableSemaphores[mCurrentSwapChainImage],
                            VK_NULL_HANDLE, &mCurrentSwapChainImage);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    std::cout << "Recreating Swapchain\n";
    // recreateSwapChain();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  vkResetFences(mLogicalDevice, 1, &mInFlightFences[mCurrentSwapChainImage]);

  // std::cout << "Current frame: " << mCurrentSwapChainImage << "\n";

  updateUniformBuffer(0);

  std::vector<VkCommandBuffer> commandBuffers = {
			mDrawingCommandBuffers[mCurrentSwapChainImage]
		};

  commandBuffers.push_back(mTextOverlay->mCommandBuffers[mCurrentSwapChainImage]);

  VulkanHelper::submitCommandBuffers(
       commandBuffers, mGraphicsQueue,
       mImageAvailableSemaphores[mCurrentSwapChainImage],
       mRenderFinishedSemaphores[mCurrentSwapChainImage], mInFlightFences[mCurrentSwapChainImage]);

  // Now present the image
  VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mCurrentSwapChainImage]};

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {mSwapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &mCurrentSwapChainImage;

  std::vector<VkResult> resultsArray;
  resultsArray.resize(mSwapChainImageCount);

  resultsArray[0] = VK_EVENT_SET;
  resultsArray[1] = VK_EVENT_SET;
  resultsArray[2] = VK_EVENT_SET;

  // presentInfo.pResults = nullptr; // Optional
  presentInfo.pResults = resultsArray.data(); // Optional
  result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    std::cout << "Need to recreate swapchain\n";
    // recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
}

} // namespace VulkanEngine