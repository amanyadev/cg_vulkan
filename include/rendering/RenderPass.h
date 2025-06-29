#pragma once

#include "core/VulkanDevice.h"
#include "rendering/SwapChain.h"
#include <vulkan/vulkan.h>

class RenderPass {
public:
    RenderPass(VulkanDevice* device, SwapChain* swapChain);
    ~RenderPass();

    VkRenderPass getRenderPass() const { return m_renderPass; }

private:
    void createRenderPass();
    void createColorAttachment(VkAttachmentDescription& colorAttachment);
    void setupSubpass(VkSubpassDescription& subpass, VkAttachmentReference& colorAttachmentRef);
    void setupDependency(VkSubpassDependency& dependency);

    VulkanDevice* m_device;
    SwapChain* m_swapChain;
    VkRenderPass m_renderPass;
};
