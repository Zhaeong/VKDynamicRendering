#include <vulkan_renderer.hpp>

namespace VulkanEngine {

VulkanRenderer::VulkanRenderer(SDL_Window *sdlWindow) {
  mWindow = sdlWindow;
  createInstance();
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
} // namespace VulkanEngine