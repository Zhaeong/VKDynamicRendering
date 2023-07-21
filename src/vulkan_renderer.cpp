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
}
VulkanRenderer::~VulkanRenderer() {}

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

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = i;
    queueCreateInfo.queueCount = 1;

    queueCreateInfo.pQueuePriorities = &queuePriority;
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
  VkPhysicalDeviceVulkan12Features vk12Features{};
  vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  createInfo.pNext = &vk12Features;

  if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicalDevice) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(mLogicalDevice, indices.graphicsFamily, 0, &mGraphicsQueue);

  // Now create the present queue
  vkGetDeviceQueue(mLogicalDevice, indices.presentFamily, 0, &mPresentQueue);
}
} // namespace VulkanEngine