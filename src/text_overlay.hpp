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
        std::vector<VkImageView> mSwapChainImageViews;
        VkFormat mSwapChainImageFormat;
        VkExtent2D mSwapChainExtent;
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
        // Pointer to mapped vertex buffer
	    glm::vec4 *mMappedVertexBufferMemory = nullptr;
        uint32_t mNumLetters;
        float mScale = 1.0f;
	    stb_fontchar mSTBFontData[STB_FONT_consolas_24_latin1_NUM_CHARS];

    public:
        enum TextAlign { alignLeft, alignCenter, alignRight };
        std::vector<VkCommandBuffer> mCommandBuffers;


        TextOverlay(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t graphicsFamilyIndex, std::vector<VkImageView> swapChainImageViews, VkFormat swapChainImageFormat, VkExtent2D swapChainExtent, VkQueue queue);
        ~TextOverlay();

        void prepareResources();
        void preparePipeline();

        void beginTextUpdate();
        void addText(std::string text, float x, float y, TextAlign align);
        void endTextUpdate();

        void updateCommandBuffers();
};