#pragma once

#include "core/VulkanDevice.h"
#include <vulkan/vulkan.h>
#include <vector>

class CommandBuffer {
public:
    CommandBuffer(VulkanDevice* device);
    ~CommandBuffer();

    void createCommandPool();
    void createCommandBuffers(VkRenderPass renderPass, const std::vector<VkFramebuffer>& framebuffers,
                            VkExtent2D extent, VkPipeline graphicsPipeline, VkPipelineLayout pipelineLayout,
                            VkDescriptorSet descriptorSet);
    void recordCommandBuffer(size_t index, VkRenderPass renderPass, VkFramebuffer framebuffer,
                           VkExtent2D extent, VkPipeline graphicsPipeline, VkPipelineLayout pipelineLayout,
                           VkDescriptorSet descriptorSet, class DebugUI* debugUI = nullptr);
    VkCommandBuffer getCommandBuffer(size_t index) const { return m_commandBuffers[index]; }
    size_t size() const { return m_commandBuffers.size(); }

private:
    void cleanup();

    VulkanDevice* m_device;
    VkCommandPool m_commandPool{VK_NULL_HANDLE};
    std::vector<VkCommandBuffer> m_commandBuffers;
};
