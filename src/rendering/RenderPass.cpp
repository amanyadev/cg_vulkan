#include "rendering/RenderPass.h"
#include "core/VulkanDevice.h"
#include <stdexcept>

RenderPass::RenderPass(VulkanDevice* device, SwapChain* swapChain)
    : m_device(device), m_swapChain(swapChain), m_renderPass(VK_NULL_HANDLE) {
    createRenderPass();
}

RenderPass::~RenderPass() {
    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device->getDevice(), m_renderPass, nullptr);
    }
}

void RenderPass::createColorAttachment(VkAttachmentDescription& colorAttachment) {
    colorAttachment = {};
    colorAttachment.format = m_swapChain->getImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

void RenderPass::setupSubpass(VkSubpassDescription& subpass, VkAttachmentReference& colorAttachmentRef) {
    colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
}

void RenderPass::setupDependency(VkSubpassDependency& dependency) {
    dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
}

void RenderPass::createRenderPass() {
    VkAttachmentDescription colorAttachment;
    createColorAttachment(colorAttachment);

    VkAttachmentReference colorAttachmentRef;
    VkSubpassDescription subpass;
    setupSubpass(subpass, colorAttachmentRef);

    VkSubpassDependency dependency;
    setupDependency(dependency);

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device->getDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }
}
