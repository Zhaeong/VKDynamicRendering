#include <text_overlay.hpp>


TextOverlay::TextOverlay(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t graphicsFamilyIndex, std::vector<VkImage> swapChainImages, VkQueue queue) {
    mPhysicalDevice = physicalDevice;
    mLogicalDevice = logicalDevice;
    mGraphicsFamilyIndex = graphicsFamilyIndex;
    mSwapChainImages = swapChainImages;
    mQueue = queue;

    mCommandBuffers.resize(mSwapChainImages.size());

    prepareResources();
}

TextOverlay::~TextOverlay(){
    vkDestroyBuffer(mLogicalDevice, mVertexBuffer, nullptr);
	vkFreeMemory(mLogicalDevice, mVertexBufferMemory, nullptr);
    vkDestroyCommandPool(mLogicalDevice, mCommandPool, nullptr);
}

void TextOverlay::prepareResources(){

    const uint32_t fontWidth = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;
	const uint32_t fontHeight = STB_FONT_consolas_24_latin1_BITMAP_HEIGHT;

	static unsigned char font24pixels[fontHeight][fontWidth];
	stb_font_consolas_24_latin1(mSTBFontData, font24pixels, fontHeight);

    //Command buffer
    VkCommandPoolCreateInfo poolInfo = VulkanInit::command_pool_create_info();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = mGraphicsFamilyIndex;
    VK_CHECK(vkCreateCommandPool(mLogicalDevice, &poolInfo, nullptr, &mCommandPool), "TextOverlay vkCreateCommandPool");

    VkCommandBufferAllocateInfo allocInfo = VulkanInit::command_buffer_allocate_info(mCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, (uint32_t)mCommandBuffers.size());
    VK_CHECK(vkAllocateCommandBuffers(mLogicalDevice, &allocInfo, mCommandBuffers.data()), "TextOverlay vkAllocateCommandBuffers");

    //Vertex buffer
    VkDeviceSize bufferSize = TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4);
    VkBufferCreateInfo bufferInfo = VulkanInit::buffer_create_info(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bufferSize);
    VK_CHECK(vkCreateBuffer(mLogicalDevice, &bufferInfo, nullptr, &mVertexBuffer), "TextOverlay vkCreateBuffer");

    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAllocInfo = VulkanInit::memory_allocate_info();
    vkGetBufferMemoryRequirements(mLogicalDevice, mVertexBuffer, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = VulkanHelper::findMemoryType(mPhysicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK(vkAllocateMemory(mLogicalDevice, &memAllocInfo, nullptr, &mVertexBufferMemory), "vkAllocateMemory");
    VK_CHECK(vkBindBufferMemory(mLogicalDevice, mVertexBuffer, mVertexBufferMemory, 0), "vkBindBufferMemory");

    // Font texture
    VkImageCreateInfo imageInfo = VulkanInit::image_create_info();
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8_UNORM;
    imageInfo.extent.width = fontWidth;
    imageInfo.extent.height = fontHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK(vkCreateImage(mLogicalDevice, &imageInfo, nullptr, &mImage), "vkCreateImage");

    vkGetImageMemoryRequirements(mLogicalDevice, mImage, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = VulkanHelper::findMemoryType(mPhysicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK(vkAllocateMemory(mLogicalDevice, &memAllocInfo, nullptr, &mImageMemory),"vkAllocateMemory");
	VK_CHECK(vkBindImageMemory(mLogicalDevice, mImage, mImageMemory, 0),"vkBindImageMemory");

    // Staging

    struct {
        VkDeviceMemory memory;
        VkBuffer buffer;
    } stagingBuffer;

    VkBufferCreateInfo bufferCreateInfo = VulkanInit::buffer_create_info(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, memAllocInfo.allocationSize);
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(mLogicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer.buffer), "vkCreateBuffer");

    // Get memory requirements for the staging buffer (alignment, memory type bits)
    vkGetBufferMemoryRequirements(mLogicalDevice, stagingBuffer.buffer, &memReqs);

    memAllocInfo.allocationSize = memReqs.size;
    // Get memory type index for a host visible buffer
    memAllocInfo.memoryTypeIndex = VulkanHelper::findMemoryType(mPhysicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK(vkAllocateMemory(mLogicalDevice, &memAllocInfo, nullptr, &stagingBuffer.memory), "vkAllocateMemory");
    VK_CHECK(vkBindBufferMemory(mLogicalDevice, stagingBuffer.buffer, stagingBuffer.memory, 0), "vkBindBufferMemory");

    uint8_t *data;
    VK_CHECK(vkMapMemory(mLogicalDevice, stagingBuffer.memory, 0, memAllocInfo.allocationSize, 0, (void **)&data), "vkMapMemory");
    // Size of the font texture is WIDTH * HEIGHT * 1 byte (only one channel)
    memcpy(data, &font24pixels[0][0], fontWidth * fontHeight);
    vkUnmapMemory(mLogicalDevice, stagingBuffer.memory);

    // Copy to image
    VkCommandBuffer copyImageCommand = VulkanHelper::beginSingleTimeCommands(mLogicalDevice, mCommandPool);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    VulkanHelper::setImageLayout(
        copyImageCommand, 
        mImage, 
        VK_IMAGE_LAYOUT_UNDEFINED, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        subresourceRange, 
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel = 0;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = fontWidth;
    bufferCopyRegion.imageExtent.height = fontHeight;
    bufferCopyRegion.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
			copyImageCommand,
			stagingBuffer.buffer,
			mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferCopyRegion
			);
    // Prepare for shader read
	VulkanHelper::setImageLayout(
        copyImageCommand,
        mImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        subresourceRange,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	VK_CHECK(vkEndCommandBuffer(copyImageCommand), "vkEndCommandBuffer");

    VkSubmitInfo submitInfo = VulkanInit::submit_info();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyImageCommand;

    VK_CHECK(vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE), "vkQueueSubmit");
    VK_CHECK(vkQueueWaitIdle(mQueue), "vkQueueWaitIdle");

    vkFreeCommandBuffers(mLogicalDevice, mCommandPool, 1, &copyImageCommand);
    vkFreeMemory(mLogicalDevice, stagingBuffer.memory, nullptr);
    vkDestroyBuffer(mLogicalDevice, stagingBuffer.buffer, nullptr);

    // View
    VkImageViewCreateInfo imageViewInfo = VulkanInit::image_view_create_info();
    imageViewInfo.image = mImage;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = imageInfo.format;
    imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    VK_CHECK(vkCreateImageView(mLogicalDevice, &imageViewInfo, nullptr, &mImageView), "vkCreateImageView");

    // Sampler
    VkSamplerCreateInfo samplerInfo = VulkanInit::sampler_create_info();
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK(vkCreateSampler(mLogicalDevice, &samplerInfo, nullptr, &mSampler), "vkCreateSampler");


    // Descriptor
    // Font uses a separate descriptor pool
    std::vector<VkDescriptorPoolSize> poolSizes{};

    VkDescriptorPoolSize poolSizeIMGSampler{};
    poolSizeIMGSampler.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizeIMGSampler.descriptorCount = 1;
    poolSizes.push_back(poolSizeIMGSampler);

    VkDescriptorPoolCreateInfo descriptorPoolInfo = VulkanInit::descriptor_pool_create_info(static_cast<uint32_t>(poolSizes.size()), poolSizes.data(), 1);

    VK_CHECK(vkCreateDescriptorPool(mLogicalDevice, &descriptorPoolInfo, nullptr, &mDescriptorPool), "vkCreateDescriptorPool");

    // Descriptor set layout
    std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings = {
      VulkanInit::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
	};

    VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info =
	    VulkanInit::descriptor_set_layout_create_info(set_layout_bindings.data(), static_cast<uint32_t>(set_layout_bindings.size()));

    VK_CHECK(vkCreateDescriptorSetLayout(mLogicalDevice, &descriptor_layout_create_info, nullptr, &mDescriptorSetLayout), "vkCreateDescriptorSetLayout");

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_create_info =
	    VulkanInit::pipeline_layout_create_info(
	        &mDescriptorSetLayout,
	        1);

    VK_CHECK(vkCreatePipelineLayout(mLogicalDevice, &pipeline_layout_create_info, nullptr, &mPipelineLayout), "vkCreatePipelineLayout");

    // Descriptor set
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo =
	    VulkanInit::descriptor_set_allocate_info(
	        mDescriptorPool,
	        &mDescriptorSetLayout,
	        1);

    VK_CHECK(vkAllocateDescriptorSets(mLogicalDevice, &descriptorSetAllocInfo, &mDescriptorSet), "vkAllocateDescriptorSets");

    // Write to descriptor
    VkDescriptorImageInfo texture_descriptor = VulkanInit::create_descriptor_texture_raw(mSampler, mImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkWriteDescriptorSet> write_descriptor_sets = {
          VulkanInit::write_descriptor_set_from_image(mDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &texture_descriptor)
      };

    vkUpdateDescriptorSets(mLogicalDevice, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, NULL);

    // Pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK(vkCreatePipelineCache(mLogicalDevice, &pipelineCacheCreateInfo, nullptr, &mPipelineCache), "vkCreatePipelineCache");


}