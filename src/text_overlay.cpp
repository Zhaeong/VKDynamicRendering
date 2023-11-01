#include <text_overlay.hpp>


TextOverlay::TextOverlay(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t graphicsFamilyIndex, std::vector<VkImageView> swapChainImageViews, VkFormat swapChainImageFormat, VkExtent2D swapChainExtent, VkQueue queue) {
    mPhysicalDevice = physicalDevice;
    mLogicalDevice = logicalDevice;
    mGraphicsFamilyIndex = graphicsFamilyIndex;
    mSwapChainImageViews = swapChainImageViews;
    mSwapChainImageFormat = swapChainImageFormat;
    mSwapChainExtent = swapChainExtent;
    mQueue = queue;

    mCommandBuffers.resize(mSwapChainImageViews.size());


    fread(ttf_buffer, 1, 1<<20, fopen("c:/windows/fonts/times.ttf", "rb"));
    stbtt_BakeFontBitmap(ttf_buffer,0, 64.0, temp_bitmap,1024,1024, 32,96, cdata);

    std::cout << "test 0\n";

    prepareResources();
    preparePipeline();
}

TextOverlay::~TextOverlay(){

    vkDestroySampler(mLogicalDevice, mSampler, nullptr);
    vkDestroyImage(mLogicalDevice, mImage, nullptr);
    vkDestroyImageView(mLogicalDevice, mImageView, nullptr);
    vkFreeMemory(mLogicalDevice, mImageMemory, nullptr);
    vkDestroyDescriptorSetLayout(mLogicalDevice, mDescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(mLogicalDevice, mDescriptorPool, nullptr);

    vkDestroyPipelineLayout(mLogicalDevice, mPipelineLayout, nullptr);
    vkDestroyPipelineCache(mLogicalDevice, mPipelineCache, nullptr);
    vkDestroyPipeline(mLogicalDevice, mPipeline, nullptr);

    vkDestroyBuffer(mLogicalDevice, mVertexBuffer, nullptr);
	vkFreeMemory(mLogicalDevice, mVertexBufferMemory, nullptr);
    vkDestroyCommandPool(mLogicalDevice, mCommandPool, nullptr);
}

void TextOverlay::prepareResources(){

    // const uint32_t fontWidth = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;
	// const uint32_t fontHeight = STB_FONT_consolas_24_latin1_BITMAP_HEIGHT;

    const uint32_t fontWidth = 1024;
	const uint32_t fontHeight = 1024;

	// static unsigned char font24pixels[STB_FONT_consolas_24_latin1_BITMAP_HEIGHT][STB_FONT_consolas_24_latin1_BITMAP_WIDTH];
	// stb_font_consolas_24_latin1(mSTBFontData, font24pixels, fontHeight);

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

    // memcpy(data, &font24pixels[0][0], fontWidth * fontHeight);

    memcpy(data, &temp_bitmap[0], fontWidth * fontHeight);

    std::cout << "test 2\n";
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

void TextOverlay::preparePipeline() {

    //Shader
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::string vertShaderPath = "shaders/text.vert.spv";
    std::string fragShaderPath = "shaders/text.frag.spv";
    auto vertShaderCode = VulkanHelper::readFile(vertShaderPath);
    auto fragShaderCode = VulkanHelper::readFile(fragShaderPath);

    shaderStages.push_back(VulkanHelper::loadShader(mLogicalDevice, vertShaderCode, VK_SHADER_STAGE_VERTEX_BIT));
    shaderStages.push_back(VulkanHelper::loadShader(mLogicalDevice, fragShaderCode, VK_SHADER_STAGE_FRAGMENT_BIT));


    // Enable blending, using alpha from red channel of the font texture (see text.frag)
    VkPipelineColorBlendAttachmentState blendAttachmentState{};
    blendAttachmentState.blendEnable = VK_TRUE;
    blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = VulkanInit::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterizationState = VulkanInit::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
    VkPipelineColorBlendStateCreateInfo    colorBlendState = VulkanInit::pipeline_colorblend_state_create_info(1, &blendAttachmentState);
    VkPipelineDepthStencilStateCreateInfo  depthStencilState = VulkanInit::pipeline_depthstencil_state_create_info(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
    VkPipelineViewportStateCreateInfo      viewportState = VulkanInit::pipeline_viewport_state_create_Info(1, 1, 0);
    VkPipelineMultisampleStateCreateInfo   multisampleState = VulkanInit::pipeline_multisample_state_create_info(VK_SAMPLE_COUNT_1_BIT, 0);
    std::vector<VkDynamicState>            dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo       dynamicState = VulkanInit::pipeline_dynamic_state_create_info(dynamicStateEnables);

    const std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
        VulkanInit::vertex_input_binding_description(0, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX),
        VulkanInit::vertex_input_binding_description(1, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX),
    };
    const std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
        VulkanInit::vertex_input_attribute_description(0, 0, VK_FORMAT_R32G32_SFLOAT, 0),					// Location 0: Position
        VulkanInit::vertex_input_attribute_description(1, 1, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec2)),	// Location 1: UV
    };

    VkPipelineVertexInputStateCreateInfo vertexInputState = VulkanInit::pipeline_vertex_input_state_create_info(vertexInputBindings, vertexInputAttributes);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = mPipelineLayout;
    pipelineCreateInfo.renderPass = nullptr;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.basePipelineIndex = -1;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    //================================================================================================
    // For dynamic rendering
    //================================================================================================
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info{};
    pipeline_rendering_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;

    pipeline_rendering_create_info.colorAttachmentCount = 1;
    pipeline_rendering_create_info.pColorAttachmentFormats = &mSwapChainImageFormat;

    pipeline_rendering_create_info.depthAttachmentFormat =
        VulkanHelper::findSupportedFormat(
            mPhysicalDevice,
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    pipeline_rendering_create_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    pipelineCreateInfo.pNext = &pipeline_rendering_create_info;

    VK_CHECK(vkCreateGraphicsPipelines(mLogicalDevice, mPipelineCache, 1, &pipelineCreateInfo, nullptr, &mPipeline), "vkCreateGraphicsPipelines");


}


// Map buffer
void TextOverlay::beginTextUpdate()
{
    VK_CHECK(vkMapMemory(mLogicalDevice, mVertexBufferMemory, 0, VK_WHOLE_SIZE, 0, (void **)&mMappedVertexBufferMemory), "vkMapMemory");
    mNumLetters = 0;
}

// Add text to the current buffer
void TextOverlay::addText(std::string text, float x, float y, TextAlign align)
{
    const uint32_t firstChar = STB_FONT_consolas_24_latin1_FIRST_CHAR;

    assert(mMappedVertexBufferMemory != nullptr);

    const float charW = 1.5f * mScale / mSwapChainExtent.width;
    const float charH = 1.5f * mScale / mSwapChainExtent.height;

    float fbW = (float)mSwapChainExtent.width;
    float fbH = (float)mSwapChainExtent.height;

    //Converting pixel space to normalized device coordinates
    x = (x / fbW * 2.0f) - 1.0f;
    y = (y / fbH * 2.0f) - 1.0f;

    // Calculate text width
    float textWidth = 0;
    for (auto letter : text)
    {
        // stb_fontchar *charData = &mSTBFontData[(uint32_t)letter - firstChar];
        stbtt_bakedchar *charData = &cdata[(uint32_t)letter - firstChar];
        textWidth += charData->xadvance * charW;
    }

    switch (align)
    {
        case alignRight:
            x -= textWidth;
            break;
        case alignCenter:
            x -= textWidth / 2.0f;
            break;
        case alignLeft:
            break;
    }

    // Generate a uv mapped quad per char in the new text
    for (auto letter : text)
    {
        std::cout << "=======================================Letter: " << letter << ":" << (uint32_t)letter << "===========\n";
        stbtt_aligned_quad charQuad;
        float xPos, yPos;        
        stbtt_GetBakedQuad(cdata, 1024,1024, (uint32_t)letter-32, &xPos,&yPos,&charQuad,1);//1=opengl & d3d10+,0=d3d9

        stbtt_bakedchar *charData = &cdata[(uint32_t)letter - firstChar];

        mMappedVertexBufferMemory->x = (x + (float)charData->x0 * charW);
        mMappedVertexBufferMemory->y = (y + (float)charData->y0 * charH);
        mMappedVertexBufferMemory->z = charQuad.s0;
        mMappedVertexBufferMemory->w = charQuad.t0;
        std::cout << "1: x:" << mMappedVertexBufferMemory->x << " y:" << mMappedVertexBufferMemory->y << " z:" << mMappedVertexBufferMemory->z << " w:"<< mMappedVertexBufferMemory->w  << "\n";
        mMappedVertexBufferMemory++; 

        mMappedVertexBufferMemory->x = (x + (float)charData->x1 * charW);
        mMappedVertexBufferMemory->y = (y + (float)charData->y0 * charH);
        mMappedVertexBufferMemory->z = charQuad.s1;
        mMappedVertexBufferMemory->w = charQuad.t0;
        mMappedVertexBufferMemory++; 

        mMappedVertexBufferMemory->x = (x + (float)charData->x0 * charW);
        mMappedVertexBufferMemory->y = (y + (float)charData->y1 * charH);
        mMappedVertexBufferMemory->z = charQuad.s0;
        mMappedVertexBufferMemory->w = charQuad.t1;
        mMappedVertexBufferMemory++; 

        mMappedVertexBufferMemory->x = (x + (float)charData->x1 * charW);
        mMappedVertexBufferMemory->y = (y + (float)charData->y1 * charH);
        mMappedVertexBufferMemory->z = charQuad.s1;
        mMappedVertexBufferMemory->w = charQuad.t1;
        // std::cout << "1: x:" << mMappedVertexBufferMemory->x << " y:" << mMappedVertexBufferMemory->y << " z:" << mMappedVertexBufferMemory->z << " w:"<< mMappedVertexBufferMemory->w  << "\n";
        mMappedVertexBufferMemory++; 

        x += charData->xadvance * charW;



        ///////////////////////////////////////


        // stb_fontchar *charData = &mSTBFontData[(uint32_t)letter - firstChar];
        // std::cout << "=======================================Letter: " << letter << "===========\n";
        // mMappedVertexBufferMemory->x = (x + (float)charData->x0 * charW);
        // mMappedVertexBufferMemory->y = (y + (float)charData->y0 * charH);
        // mMappedVertexBufferMemory->z = charData->s0;
        // mMappedVertexBufferMemory->w = charData->t0;
        // std::cout << "1: x:" << mMappedVertexBufferMemory->x << " y:" << mMappedVertexBufferMemory->y << " z:" << mMappedVertexBufferMemory->z << " w:"<< mMappedVertexBufferMemory->w  << "\n";
        // mMappedVertexBufferMemory++;        

        // mMappedVertexBufferMemory->x = (x + (float)charData->x1 * charW);
        // mMappedVertexBufferMemory->y = (y + (float)charData->y0 * charH);
        // mMappedVertexBufferMemory->z = charData->s1;
        // mMappedVertexBufferMemory->w = charData->t0;
        // std::cout << "2: x:" << mMappedVertexBufferMemory->x << " y:" << mMappedVertexBufferMemory->y << " z:" << mMappedVertexBufferMemory->z << " w:"<< mMappedVertexBufferMemory->w  << "\n";
        // mMappedVertexBufferMemory++;

        // mMappedVertexBufferMemory->x = (x + (float)charData->x0 * charW);
        // mMappedVertexBufferMemory->y = (y + (float)charData->y1 * charH);
        // mMappedVertexBufferMemory->z = charData->s0;
        // mMappedVertexBufferMemory->w = charData->t1;
        // std::cout << "3: x:" << mMappedVertexBufferMemory->x << " y:" << mMappedVertexBufferMemory->y << " z:" << mMappedVertexBufferMemory->z << " w:"<< mMappedVertexBufferMemory->w  << "\n";
        // mMappedVertexBufferMemory++;

        // mMappedVertexBufferMemory->x = (x + (float)charData->x1 * charW);
        // mMappedVertexBufferMemory->y = (y + (float)charData->y1 * charH);
        // mMappedVertexBufferMemory->z = charData->s1;
        // mMappedVertexBufferMemory->w = charData->t1;
        // std::cout << "4: x:" << mMappedVertexBufferMemory->x << " y:" << mMappedVertexBufferMemory->y << " z:" << mMappedVertexBufferMemory->z << " w:"<< mMappedVertexBufferMemory->w  << "\n";
        // mMappedVertexBufferMemory++;

        // x += charData->advance * charW;

        mNumLetters++;
    }
}

// Unmap buffer and update command buffers
void TextOverlay::endTextUpdate()
{
    vkUnmapMemory(mLogicalDevice, mVertexBufferMemory);
    mMappedVertexBufferMemory = nullptr;
    updateCommandBuffers();
}

// Needs to be called by the application
void TextOverlay::updateCommandBuffers()
{
    VkCommandBufferBeginInfo beginInfo = VulkanInit::command_buffer_begin_info();

    for (int32_t i = 0; i < mCommandBuffers.size(); ++i)
    {

        vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo);

        VkRenderingAttachmentInfoKHR renderingColorAttachmentInfo{};
        renderingColorAttachmentInfo.sType =
            VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        renderingColorAttachmentInfo.imageView =
            mSwapChainImageViews[i];
        renderingColorAttachmentInfo.imageLayout =
            VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        renderingColorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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

        vkCmdBeginRendering(mCommandBuffers[i], &renderingInfo);

        VkViewport viewport = VulkanInit::viewport((float)mSwapChainExtent.width, (float)mSwapChainExtent.height, 0.0f, 1.0f);
        vkCmdSetViewport(mCommandBuffers[i], 0, 1, &viewport);

        VkRect2D scissor = VulkanInit::rect2D(mSwapChainExtent.width, mSwapChainExtent.height, 0, 0);
        vkCmdSetScissor(mCommandBuffers[i], 0, 1, &scissor);

        vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
        vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSet, 0, NULL);

        VkDeviceSize offsets = 0;
        //First two is pos, second two is uv, so it renders 4
        vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, &mVertexBuffer, &offsets);
        vkCmdBindVertexBuffers(mCommandBuffers[i], 1, 1, &mVertexBuffer, &offsets);
        for (uint32_t j = 0; j < mNumLetters; j++)
        {
            vkCmdDraw(mCommandBuffers[i], 4, 1, j * 4, 0);
        }

        vkCmdEndRendering(mCommandBuffers[i]);

        VK_CHECK(vkEndCommandBuffer(mCommandBuffers[i]), "vkEndCommandBuffer");
    }
}
