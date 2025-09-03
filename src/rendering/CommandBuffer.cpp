#include "rendering/CommandBuffer.h"
#include "ui/DebugUI.h"
#include <stdexcept>
#include <array>

CommandBuffer::CommandBuffer(VulkanDevice* device) : m_device(device) {
    createCommandPool();
}

CommandBuffer::~CommandBuffer() {
    cleanup();
}

void CommandBuffer::cleanup() {
    if (m_commandPool != VK_NULL_HANDLE) {
        if (!m_commandBuffers.empty()) {
            vkFreeCommandBuffers(m_device->getDevice(), m_commandPool,
                static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        }
        vkDestroyCommandPool(m_device->getDevice(), m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }
    m_commandBuffers.clear();
}

void CommandBuffer::createCommandPool() {
    VulkanDevice::QueueFamilyIndices queueFamilyIndices = m_device->findQueueFamilies(m_device->getPhysicalDevice());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_device->getDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void CommandBuffer::createCommandBuffers(VkRenderPass renderPass,
    const std::vector<VkFramebuffer>& framebuffers, VkExtent2D extent, VkPipeline graphicsPipeline,
    VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet) {

    m_commandBuffers.resize(framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device->getDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    // Record command buffers
    for (size_t i = 0; i < m_commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;

        VkClearValue clearColor = {{{0.95f, 0.95f, 0.95f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Bind the graphics pipeline
        vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        // Bind descriptor set
        vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        // No drawing commands here - actual rendering is done in recordCommandBuffer method

        vkCmdEndRenderPass(m_commandBuffers[i]);

        if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }
}

void CommandBuffer::recordCommandBuffer(size_t index, VkRenderPass renderPass, VkFramebuffer framebuffer,
                                       VkExtent2D extent, VkPipeline graphicsPipeline, VkPipelineLayout pipelineLayout,
                                       VkDescriptorSet descriptorSet, DebugUI* debugUI, GLTFViewer* viewer) {
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(m_commandBuffers[index], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.95f, 0.95f, 0.95f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_commandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind the graphics pipeline
    vkCmdBindPipeline(m_commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // Render model through GLTFViewer if provided
    if (viewer && viewer->hasModel()) {
        // Bind GLTFViewer's descriptor set which has proper camera matrices
        VkDescriptorSet viewerDescriptorSet = viewer->getDescriptorSet();
        vkCmdBindDescriptorSets(m_commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, 0, 1, &viewerDescriptorSet, 0, nullptr);
        
        viewer->renderToCommandBuffer(m_commandBuffers[index], pipelineLayout);
    } else {
        // Fallback to pipeline descriptor set if no model
        vkCmdBindDescriptorSets(m_commandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    }

    // Render ImGui if provided
    if (debugUI) {
        debugUI->renderDrawData(m_commandBuffers[index]);
    }

    vkCmdEndRenderPass(m_commandBuffers[index]);

    if (vkEndCommandBuffer(m_commandBuffers[index]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}
