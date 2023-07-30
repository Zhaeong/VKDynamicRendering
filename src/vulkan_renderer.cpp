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

  createCommandPool();
  createCommandBuffers(MAX_FRAMES_IN_FLIGHT);
  createSyncObjects(MAX_FRAMES_IN_FLIGHT);

  createSwapChain(mSurface);
  createSwapChainImageViews();

  createGraphicsPipeline();

  // Creation of custom input objects into the graphics pipeline
  std::vector<Utils::Vertex> vertices = {
      {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

      {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};

  createVertexBuffer(vertices);
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

  VkInstanceCreateInfo createInfo = VulkanInit::instance_create_info(
      appInfo, requiredVkExtenstionsForSDL.size(),
      requiredVkExtenstionsForSDL.data(), enabledLayerCount, enabledLayerNames);

  if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }
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
  }
}

void VulkanRenderer::createLogicalDevice() {

  // Specifying queues to be created
  Utils::QueueFamilyIndices indices =
      VulkanHelper::iFindQueueFamilies(mPhysicalDevice, mSurface);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

  // set will only contain unique values
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily,
                                            indices.presentFamily};

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

  vkGetDeviceQueue(mLogicalDevice, indices.graphicsFamily, 0, &mGraphicsQueue);

  // Now create the present queue
  vkGetDeviceQueue(mLogicalDevice, indices.presentFamily, 0, &mPresentQueue);
}

void VulkanRenderer::createCommandPool() {
  Utils::QueueFamilyIndices queueFamilyIndices =
      VulkanHelper::iFindQueueFamilies(mPhysicalDevice, mSurface);

  VkCommandPoolCreateInfo poolInfo = VulkanInit::command_pool_create_info();
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

  if (vkCreateCommandPool(mLogicalDevice, &poolInfo, nullptr, &mCommandPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}

void VulkanRenderer::createCommandBuffers(uint32_t number) {

  mCommandBuffers.resize(number);

  VkCommandBufferAllocateInfo allocInfo =
      VulkanInit::command_buffer_allocate_info(
          mCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          (uint32_t)mCommandBuffers.size());

  if (vkAllocateCommandBuffers(mLogicalDevice, &allocInfo,
                               mCommandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
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

  Utils::QueueFamilyIndices indices =
      VulkanHelper::iFindQueueFamilies(mPhysicalDevice, surface);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily,
                                   indices.presentFamily};

  if (indices.graphicsFamily != indices.presentFamily) {
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

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  // Fragment Shader
  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  // Shader stage createInfo
  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  PIPElineInfo.stageCount = 2;
  PIPElineInfo.pStages = shaderStages;
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

  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

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
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  // Descriptor set layouts
  pipelineLayoutInfo.setLayoutCount = 1; // Optional

  VkDescriptorSetLayout descriptorSetLayout =
      VulkanInit::create_descriptorSetLayout(mLogicalDevice);
  pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional

  // Push constants
  pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

  if (vkCreatePipelineLayout(mLogicalDevice, &pipelineLayoutInfo, nullptr,
                             &mPipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

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
  if (vkCreateGraphicsPipelines(mLogicalDevice, VK_NULL_HANDLE, 1,
                                &PIPElineInfo, nullptr,
                                &mGraphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  // Cleanup===========
  vkDestroyShaderModule(mLogicalDevice, fragShaderModule, nullptr);
  vkDestroyShaderModule(mLogicalDevice, vertShaderModule, nullptr);
  //================================================================================================
}

void VulkanRenderer::createVertexBuffer(std::vector<Utils::Vertex> vertices) {

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

} // namespace VulkanEngine