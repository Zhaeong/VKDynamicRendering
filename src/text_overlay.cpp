#include <text_overlay.hpp>


TextOverlay::TextOverlay(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t graphicsFamilyIndex, std::vector<VkImage> swapChainImages) {
    mPhysicalDevice = physicalDevice;
    mLogicalDevice = logicalDevice;
    mGraphicsFamilyIndex = graphicsFamilyIndex;
    mSwapChainImages = swapChainImages;

    mCommandBuffers.resize(mSwapChainImages.size());

    prepareResources();
}

TextOverlay::~TextOverlay(){

}

void TextOverlay::prepareResources(){
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


}