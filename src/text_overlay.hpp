#pragma once

#include <vulkan/vulkan.h>

#include <vulkan_helper.hpp>
#include <vulkan_initializers.hpp>

#define TEXTOVERLAY_MAX_CHAR_COUNT 2048
class TextOverlay {
    private:
        //Required to pass in
        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
        VkDevice mLogicalDevice;
        uint32_t mGraphicsFamilyIndex;
        std::vector<VkImage> mSwapChainImages;

        //Created by object
        VkCommandPool mCommandPool;
        VkBuffer mVertexBuffer;
        VkDeviceMemory mVertexBufferMemory;

    public:

        std::vector<VkCommandBuffer> mCommandBuffers;
        TextOverlay(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t graphicsFamilyIndex, std::vector<VkImage> swapChainImages);
        ~TextOverlay();

        void prepareResources();

};