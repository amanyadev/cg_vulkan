#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "VulkanDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

class GraphicsPipeline {
public:
    GraphicsPipeline(VulkanDevice* device, SwapChain* swapChain);
    ~GraphicsPipeline();

    VkRenderPass getRenderPass() const { return m_renderPass->getRenderPass(); }
    VkPipeline getPipeline() const { return m_graphicsPipeline; }
    VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
    Framebuffer* getFramebuffer() const { return m_framebuffer.get(); }

private:
    void createPipeline();
    void createGraphicsPipeline();
    static std::vector<char> readShaderFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    VulkanDevice* m_device;
    SwapChain* m_swapChain;
    std::unique_ptr<RenderPass> m_renderPass;
    std::unique_ptr<Framebuffer> m_framebuffer;
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
    VkPipeline m_graphicsPipeline{VK_NULL_HANDLE};
};

#endif // GRAPHICS_PIPELINE_H
