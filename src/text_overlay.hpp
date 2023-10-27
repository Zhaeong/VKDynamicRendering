#pragma once

#include <vulkan/vulkan.h>

#include <vulkan_helper.hpp>
#include <vulkan_initializers.hpp>

#include <stb_font_consolas_24_latin1.inl>

#define TEXTOVERLAY_MAX_CHAR_COUNT 2048
class TextOverlay {
    private:
        //Required to pass in
        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
        VkDevice mLogicalDevice;
        uint32_t mGraphicsFamilyIndex;
        std::vector<VkImage> mSwapChainImages;
        VkFormat mSwapChainImageFormat;
        VkQueue mQueue;

        //Created by object
        VkCommandPool mCommandPool;
        VkBuffer mVertexBuffer;
        VkDeviceMemory mVertexBufferMemory;

        //Vulkan Image
	    VkImage mImage;
        VkDeviceMemory mImageMemory;
	    VkImageView mImageView;
        VkSampler mSampler;

        VkDescriptorPool mDescriptorPool;
	    VkDescriptorSetLayout mDescriptorSetLayout;
	    VkDescriptorSet mDescriptorSet;

        VkPipelineLayout mPipelineLayout;
	    VkPipelineCache mPipelineCache;
	    VkPipeline mPipeline;

        // Pointer to mapped vertex buffer
	    glm::vec4 *mMappedVertexBuffer = nullptr;
	    stb_fontchar mSTBFontData[STB_FONT_consolas_24_latin1_NUM_CHARS];
	    uint32_t mNumLetters;

    public:

        std::vector<VkCommandBuffer> mCommandBuffers;
        TextOverlay(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t graphicsFamilyIndex, std::vector<VkImage> swapChainImages, VkFormat swapChainImageFormat, VkQueue queue);
        ~TextOverlay();

        void prepareResources();
        void preparePipeline();

};