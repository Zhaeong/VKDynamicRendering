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

  createSwapChain();
  createSwapChainImageViews();
}
VulkanRenderer::~VulkanRenderer() {
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
      !VulkanInit::iCheckValidationLayerSupport(mValidationLayers)) {
    throw std::runtime_error("Validation Layer requested but not available\n");
  } else if (mEnableValidationLayers) {
    std::cout << "Enabling Validation Layers\n";
  } else {
    std::cout << "ValidationLayers Not enabled\n";
  }

  std::vector<const char *> requiredVkExtenstionsForSDL =
      VulkanInit::iGetRequiredVkExtensions(mWindow);

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
    if (VulkanInit::iIsDeviceSuitable(vPhysicalDevices[i], mSurface,
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
      VulkanInit::iFindQueueFamilies(mPhysicalDevice, mSurface);

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
      VulkanInit::iFindQueueFamilies(mPhysicalDevice, mSurface);

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

void VulkanRenderer::createSwapChain() {
  Utils::SwapChainSupportDetails swapChainSupport =
      VulkanInit::iQuerySwapChainSupport(mPhysicalDevice, mSurface);

  VkSurfaceFormatKHR surfaceFormat =
      VulkanInit::iChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode =
      VulkanInit::iChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent =
      VulkanInit::iChooseSwapExtent(mWindow, swapChainSupport.capabilities);

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
  createInfo.surface = mSurface;
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
      VulkanInit::iFindQueueFamilies(mPhysicalDevice, mSurface);
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
    mSwapChainImageViews[i] = VulkanInit::createImageView(
        mLogicalDevice, mSwapChainImages[i], mSwapChainImageFormat,
        VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

} // namespace VulkanEngine