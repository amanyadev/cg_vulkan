#include "rendering/Framebuffer.h"
#include <stdexcept>

Framebuffer::Framebuffer(VulkanDevice* device, SwapChain* swapChain, VkRenderPass renderPass)
    : m_device(device), m_swapChain(swapChain), m_renderPass(renderPass) {
    createDepthResources();
    createFramebuffers();
}

Framebuffer::~Framebuffer() {
    cleanup();
}

void Framebuffer::cleanup() {
    for (auto framebuffer : m_swapChainFramebuffers) {
        vkDestroyFramebuffer(m_device->getDevice(), framebuffer, nullptr);
    }
    m_swapChainFramebuffers.clear();
    
    // Cleanup depth resources
    if (m_depthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device->getDevice(), m_depthImageView, nullptr);
    }
    if (m_depthImage != VK_NULL_HANDLE) {
        vkDestroyImage(m_device->getDevice(), m_depthImage, nullptr);
    }
    if (m_depthImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device->getDevice(), m_depthImageMemory, nullptr);
    }
}

void Framebuffer::createFramebuffers() {
    const auto& swapChainImageViews = m_swapChain->getImageViews();
    m_swapChainFramebuffers.resize(swapChainImageViews.size());

    // Create a framebuffer for each swap chain image view
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i],
            m_depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapChain->getExtent().width;
        framebufferInfo.height = m_swapChain->getExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device->getDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer");
        }
    }
}

void Framebuffer::createDepthResources() {
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    VkExtent2D swapChainExtent = m_swapChain->getExtent();

    // Create depth image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_device->getDevice(), &imageInfo, nullptr, &m_depthImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create depth image!");
    }

    // Allocate memory for depth image
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device->getDevice(), m_depthImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_device->getDevice(), &allocInfo, nullptr, &m_depthImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate depth image memory!");
    }

    vkBindImageMemory(m_device->getDevice(), m_depthImage, m_depthImageMemory, 0);

    // Create depth image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_depthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device->getDevice(), &viewInfo, nullptr, &m_depthImageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create depth image view!");
    }
}

uint32_t Framebuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_device->getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}
