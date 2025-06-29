#include "rendering/Framebuffer.h"
#include <stdexcept>

Framebuffer::Framebuffer(VulkanDevice* device, SwapChain* swapChain, VkRenderPass renderPass)
    : m_device(device), m_swapChain(swapChain), m_renderPass(renderPass) {
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
}

void Framebuffer::createFramebuffers() {
    const auto& swapChainImageViews = m_swapChain->getImageViews();
    m_swapChainFramebuffers.resize(swapChainImageViews.size());

    // Create a framebuffer for each swap chain image view
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapChain->getExtent().width;
        framebufferInfo.height = m_swapChain->getExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device->getDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer");
        }
    }
}
