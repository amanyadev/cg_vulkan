#pragma once

#include "core/VulkanDevice.h"
#include "rendering/SwapChain.h"
#include <vulkan/vulkan.h>
#include <vector>

class Framebuffer {
public:
    Framebuffer(VulkanDevice* device, SwapChain* swapChain, VkRenderPass renderPass);
    ~Framebuffer();

    VkFramebuffer getFramebuffer(size_t index) const { return m_swapChainFramebuffers[index]; }
    size_t size() const { return m_swapChainFramebuffers.size(); }

private:
    void createFramebuffers();
    void cleanup();

    VulkanDevice* m_device;
    SwapChain* m_swapChain;
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
};
