#include <vulkan_renderer.hpp>

namespace VulkanEngine {

VulkanRenderer::VulkanRenderer(SDL_Window *sdlWindow) {
  mWindow = sdlWindow;
  createInstance();

  if (SDL_Vulkan_CreateSurface(mWindow, mInstance, &mSurface) != SDL_TRUE) {
    throw std::runtime_error("failed to create window surface!");
  }

  pickPhysicalDevice();
}
VulkanRenderer::~VulkanRenderer() {}

void VulkanRenderer::createInstance() {

  std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  if (mEnableValidationLayers &&
      !VulkanInit::iCheckValidationLayerSupport(validationLayers)) {
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
    enabledLayerCount = validationLayers.size();
    enabledLayerNames = validationLayers.data();
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

  std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME,
      VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME};

  for (int i = 0; i < vPhysicalDevices.size(); i++) {
    std::cout << "Device Index: " << i << "\n";
    if (VulkanInit::iIsDeviceSuitable(vPhysicalDevices[i], mSurface,
                                      deviceExtensions)) {
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
} // namespace VulkanEngine