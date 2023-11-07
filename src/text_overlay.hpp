#pragma once

#include <vulkan/vulkan.h>

#include <vulkan_helper.hpp>
#include <vulkan_initializers.hpp>
#include <filesystem>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

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

        const int mBitmapHeight = 720;
        const int mBitmapWidth = 1280;
        const float mFontSize = 64.0f;
        const int mFirstChar = 32;
        const int mNumChar = 96;

        unsigned char mTTF_buffer[1<<20];

        // For using with stbtt_BakeFontBitmap
        // stbtt_bakedchar mCharData[96]; // ASCII 32..126 is 95 glyphs

        // For using with stbtt_PackFontRange
        stbtt_packedchar mCharData[96];

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