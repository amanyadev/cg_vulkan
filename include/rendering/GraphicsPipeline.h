#pragma once

#include "core/VulkanDevice.h"
#include "rendering/SwapChain.h"
#include "rendering/RenderPass.h"
#include "rendering/Framebuffer.h"
#include "rendering/CommandBuffer.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <memory>

class GraphicsPipeline {
public:
    GraphicsPipeline(VulkanDevice* device, SwapChain* swapChain);
    ~GraphicsPipeline();

    VkRenderPass getRenderPass() const { return m_renderPass->getRenderPass(); }
    VkPipeline getPipeline() const { return m_graphicsPipeline; }
    VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
    Framebuffer* getFramebuffer() const { return m_framebuffer.get(); }
    CommandBuffer* getCommandBuffer() const { return m_commandBuffer.get(); }

private:
    void createPipeline();
    void createGraphicsPipeline();
    void createCommandBuffers();
    static std::vector<char> readShaderFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    VulkanDevice* m_device;
    SwapChain* m_swapChain;
    std::unique_ptr<RenderPass> m_renderPass;
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::unique_ptr<CommandBuffer> m_commandBuffer;
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
    VkPipeline m_graphicsPipeline{VK_NULL_HANDLE};
};
