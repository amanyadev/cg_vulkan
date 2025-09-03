#include "rendering/GraphicsPipeline.h"
#include "rendering/RenderPass.h"
#include "rendering/Framebuffer.h"
#include "rendering/CommandBuffer.h"
#include "core/VulkanDevice.h"
#include "viewer/GLTFLoader.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

GraphicsPipeline::GraphicsPipeline(VulkanDevice* device, SwapChain* swapChain)
    : m_device(device), m_swapChain(swapChain) {
    m_renderPass    = std::make_unique<RenderPass>(device, swapChain);
    m_framebuffer   = std::make_unique<Framebuffer>(device, swapChain, m_renderPass->getRenderPass());
    m_commandBuffer = std::make_unique<CommandBuffer>(device);
    m_uniformBuffer = std::make_unique<UniformBuffer>(device, sizeof(UniformBufferObject));
    createPipeline();
    createCommandBuffers();
}

GraphicsPipeline::~GraphicsPipeline() {
    if (m_graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device->getDevice(), m_graphicsPipeline, nullptr);
    }
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device->getDevice(), m_pipelineLayout, nullptr);
    }
    m_framebuffer.reset(); // Destroy framebuffer before render pass
    m_renderPass.reset(); // Destroy render pass last
}

void GraphicsPipeline::createPipeline() {
    createGraphicsPipeline();
}

std::vector<char> GraphicsPipeline::readShaderFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filename);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule GraphicsPipeline::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }

    return shaderModule;
}

void GraphicsPipeline::createGraphicsPipeline() {
    std::filesystem::path buildDir = std::filesystem::current_path();
    auto vertShaderPath            = buildDir / "shaders" / "shader.vert.spv";
    auto fragShaderPath            = buildDir / "shaders" / "shader.frag.spv";

    std::cout << "\n=== Graphics Pipeline Creation ===\n";
    std::cout << "Current working directory: " << buildDir << std::endl;
    std::cout << "Loading vertex shader from: " << vertShaderPath << std::endl;
    std::cout << "Loading fragment shader from: " << fragShaderPath << std::endl;

    if (!std::filesystem::exists(vertShaderPath)) {
        std::cout << "Warning: Vertex shader file does not exist!" << std::endl;
    }
    if (!std::filesystem::exists(fragShaderPath)) {
        std::cout << "Warning: Fragment shader file does not exist!" << std::endl;
    }

    auto vertShaderCode = readShaderFile(vertShaderPath.string());
    std::cout << "Successfully loaded vertex shader (" << vertShaderCode.size() << " bytes)" << std::endl;

    auto fragShaderCode = readShaderFile(fragShaderPath.string());
    std::cout << "Successfully loaded fragment shader (" << fragShaderCode.size() << " bytes)" << std::endl;

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    std::cout << "Created vertex shader module" << std::endl;

    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
    std::cout << "Created fragment shader module" << std::endl;

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex input state - use glTF vertex format
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

    // Input assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and scissor
    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = (float) m_swapChain->getExtent().width;
    viewport.height   = (float) m_swapChain->getExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChain->getExtent();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable  = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Depth stencil state
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable       = VK_TRUE;
    depthStencil.depthWriteEnable      = VK_TRUE;
    depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable     = VK_FALSE;

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable   = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments    = &colorBlendAttachment;

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout descriptorSetLayout = m_uniformBuffer->getDescriptorSetLayout();
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(m_device->getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    // Create the graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = shaderStages;
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = &depthStencil;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = nullptr;
    pipelineInfo.layout              = m_pipelineLayout;
    pipelineInfo.renderPass          = m_renderPass->getRenderPass();
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(m_device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline)
        != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    std::cout << "Successfully created graphics pipeline\n" << std::endl;

    // Cleanup shader modules
    vkDestroyShaderModule(m_device->getDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device->getDevice(), vertShaderModule, nullptr);
    std::cout << "Cleaned up shader modules\n" << std::endl;
}

void GraphicsPipeline::createCommandBuffers() {
    // Get all framebuffers
    std::vector<VkFramebuffer> framebuffers;
    for (size_t i = 0; i < m_framebuffer->size(); i++) {
        framebuffers.push_back(m_framebuffer->getFramebuffer(i));
    }

    // Create and record command buffers with pipeline binding
    m_commandBuffer->createCommandBuffers(
        m_renderPass->getRenderPass(),
        framebuffers,
        m_swapChain->getExtent(),
        m_graphicsPipeline,
        m_pipelineLayout,
        m_uniformBuffer->getDescriptorSet()
    );
}
