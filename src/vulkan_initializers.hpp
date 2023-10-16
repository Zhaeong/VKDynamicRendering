#pragma once
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <utils.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace VulkanInit {

//===================================================
// Vulkan Create struct helpers
//===================================================
inline VkApplicationInfo application_info() {
  VkApplicationInfo application_info{};
  application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  application_info.pNext = NULL;
  application_info.pApplicationName = "SDLVulkanWithDyn";
  application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  application_info.pEngineName = "No Engine";
  application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  application_info.apiVersion = VK_API_VERSION_1_3;
  return application_info;
}

inline VkDeviceQueueCreateInfo
device_queue_create_info(uint32_t queueFamilyIndex, uint32_t queueCount,
                         const float *queuePriority) {
  VkDeviceQueueCreateInfo device_queue_create_info{};
  device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  device_queue_create_info.queueFamilyIndex = queueFamilyIndex;
  device_queue_create_info.queueCount = queueCount;
  device_queue_create_info.pQueuePriorities = queuePriority;
  return device_queue_create_info;
}

inline VkDeviceCreateInfo device_create_info(
    uint32_t queueCreateInfoCount, VkDeviceQueueCreateInfo *queueCreateInfo,
    uint32_t enabledLayerCount, const char *const *enabledLayerNames,
    uint32_t enabledExtensionCount, const char *const *enabledExtensionNames,
    VkPhysicalDeviceFeatures *enabledFeatures) {
  VkDeviceCreateInfo device_create_info{};

  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  device_create_info.queueCreateInfoCount = queueCreateInfoCount;
  device_create_info.pQueueCreateInfos = queueCreateInfo;

  device_create_info.enabledLayerCount = enabledLayerCount;
  device_create_info.ppEnabledLayerNames = enabledLayerNames;

  device_create_info.enabledExtensionCount = enabledExtensionCount;
  device_create_info.ppEnabledExtensionNames = enabledExtensionNames;

  device_create_info.pEnabledFeatures = enabledFeatures;

  return device_create_info;
}

inline VkCommandPoolCreateInfo command_pool_create_info() {
  VkCommandPoolCreateInfo command_pool_create_info{};
  command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  return command_pool_create_info;
}

inline VkCommandBufferAllocateInfo
command_buffer_allocate_info(VkCommandPool command_pool,
                             VkCommandBufferLevel level,
                             uint32_t buffer_count) {
  VkCommandBufferAllocateInfo command_buffer_allocate_info{};
  command_buffer_allocate_info.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_allocate_info.commandPool = command_pool;
  command_buffer_allocate_info.level = level;
  command_buffer_allocate_info.commandBufferCount = buffer_count;
  return command_buffer_allocate_info;
}
inline VkDescriptorSetLayout create_descriptorSetLayout(VkDevice device) {
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

  // Sampler
  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::vector<VkDescriptorSetLayoutBinding> bindings = {uboLayoutBinding,
                                                        samplerLayoutBinding};

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  VkDescriptorSetLayout descriptorSetLayout;

  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }

  return descriptorSetLayout;
}

inline VkBufferCreateInfo buffer_create_info(VkBufferUsageFlags usage,
                                             VkDeviceSize size) {
  VkBufferCreateInfo buffer_create_info{};
  buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_create_info.usage = usage;
  buffer_create_info.size = size;
  return buffer_create_info;
}

inline VkMemoryAllocateInfo memory_allocate_info() {
  VkMemoryAllocateInfo memory_allocation{};
  memory_allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  return memory_allocation;
}

inline VkCommandBufferBeginInfo command_buffer_begin_info() {
  VkCommandBufferBeginInfo cmdBufferBeginInfo{};
  cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  return cmdBufferBeginInfo;
}

inline void insert_image_memory_barrier(
    VkCommandBuffer command_buffer, VkImage image,
    VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
    VkImageLayout old_layout, VkImageLayout new_layout,
    VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
    VkImageSubresourceRange subresource_range) {

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.srcAccessMask = src_access_mask;
  barrier.dstAccessMask = dst_access_mask;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.image = image;
  barrier.subresourceRange = subresource_range;

  vkCmdPipelineBarrier(command_buffer, src_stage_mask, dst_stage_mask, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);
}

inline VkPipelineLayoutCreateInfo pipeline_layout_create_info(
    const VkDescriptorSetLayout *set_layouts,
    uint32_t                     set_layout_count = 1)
{
	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = set_layout_count;
	pipeline_layout_create_info.pSetLayouts    = set_layouts;
	return pipeline_layout_create_info;
}

inline VkDescriptorSetLayoutBinding descriptor_set_layout_binding(
    VkDescriptorType   type,
    VkShaderStageFlags flags,
    uint32_t           binding,
    uint32_t           count = 1)
{
	VkDescriptorSetLayoutBinding set_layout_binding{};
	set_layout_binding.descriptorType  = type;
	set_layout_binding.stageFlags      = flags;
	set_layout_binding.binding         = binding;
	set_layout_binding.descriptorCount = count;
	return set_layout_binding;
}

inline VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info(
    const VkDescriptorSetLayoutBinding *bindings,
    uint32_t                            binding_count)
{
	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
	descriptor_set_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_set_layout_create_info.pBindings    = bindings;
	descriptor_set_layout_create_info.bindingCount = binding_count;
	return descriptor_set_layout_create_info;
}

inline VkDescriptorSetAllocateInfo descriptor_set_allocate_info(
    VkDescriptorPool             descriptor_pool,
    const VkDescriptorSetLayout *set_layouts,
    uint32_t                     descriptor_set_count)
{
	VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
	descriptor_set_allocate_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_allocate_info.descriptorPool     = descriptor_pool;
	descriptor_set_allocate_info.pSetLayouts        = set_layouts;
	descriptor_set_allocate_info.descriptorSetCount = descriptor_set_count;
	return descriptor_set_allocate_info;
}

inline VkDescriptorBufferInfo create_descriptor_buffer(VkBuffer buffer, VkDeviceSize size, VkDeviceSize offset)
{
	VkDescriptorBufferInfo descriptor{};
	descriptor.buffer = buffer;
	descriptor.range  = size;
	descriptor.offset = offset;
	return descriptor;
}

inline VkDescriptorImageInfo create_descriptor_texture(Utils::Texture &texture)
{
	VkDescriptorImageInfo descriptor{};
	descriptor.sampler   = texture.sampler;
	descriptor.imageView = texture.view;
	descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	return descriptor;
}

inline VkWriteDescriptorSet write_descriptor_set_from_buffer(
    VkDescriptorSet         dst_set,
    VkDescriptorType        type,
    uint32_t                binding,
    VkDescriptorBufferInfo *buffer_info,
    uint32_t                descriptor_count = 1)
{
	VkWriteDescriptorSet write_descriptor_set{};
	write_descriptor_set.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set.dstSet          = dst_set;
	write_descriptor_set.descriptorType  = type;
	write_descriptor_set.dstBinding      = binding;
	write_descriptor_set.pBufferInfo     = buffer_info;
	write_descriptor_set.descriptorCount = descriptor_count;
	return write_descriptor_set;
}

inline VkWriteDescriptorSet write_descriptor_set_from_image(
    VkDescriptorSet        dst_set,
    VkDescriptorType       type,
    uint32_t               binding,
    VkDescriptorImageInfo *image_info,
    uint32_t               descriptor_count = 1)
{
	VkWriteDescriptorSet write_descriptor_set{};
	write_descriptor_set.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set.dstSet          = dst_set;
	write_descriptor_set.descriptorType  = type;
	write_descriptor_set.dstBinding      = binding;
	write_descriptor_set.pImageInfo      = image_info;
	write_descriptor_set.descriptorCount = descriptor_count;
	return write_descriptor_set;
}

inline VkImageCreateInfo image_create_info()
{
	VkImageCreateInfo image_create_info{};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	return image_create_info;
}

/** @brief Initialize an image memory barrier with no image transfer ownership */
inline VkImageMemoryBarrier image_memory_barrier()
{
	VkImageMemoryBarrier image_memory_barrier{};
	image_memory_barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	return image_memory_barrier;
}

inline VkSamplerCreateInfo sampler_create_info()
{
	VkSamplerCreateInfo sampler_create_info{};
	sampler_create_info.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.maxAnisotropy = 1.0f;
	return sampler_create_info;
}

inline VkImageViewCreateInfo image_view_create_info()
{
	VkImageViewCreateInfo image_view_create_info{};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	return image_view_create_info;
}

} // namespace VulkanInit