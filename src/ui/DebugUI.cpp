#include "ui/DebugUI.h"
#include "rendering/RenderPass.h"
#include <stdexcept>
#include <iostream>

DebugUI::DebugUI(VulkanDevice* device, SwapChain* swapChain, VkRenderPass renderPass, GLFWwindow* window)
    : m_device(device), m_swapChain(swapChain), m_renderPass(renderPass), m_window(window) {
    
    createDescriptorPool();
    createCommandBuffers();
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(m_window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_device->getInstance();
    init_info.PhysicalDevice = m_device->getPhysicalDevice();
    init_info.Device = m_device->getDevice();
    init_info.QueueFamily = m_device->findQueueFamilies(m_device->getPhysicalDevice()).graphicsFamily.value();
    init_info.Queue = m_device->getGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_descriptorPool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = m_swapChain->getImages().size();
    init_info.ImageCount = m_swapChain->getImages().size();
    init_info.CheckVkResultFn = nullptr;
    init_info.RenderPass = m_renderPass;
    
    ImGui_ImplVulkan_Init(&init_info);
    m_initialized = true;
}

DebugUI::~DebugUI() {
    if (m_initialized) {
        vkDeviceWaitIdle(m_device->getDevice());
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device->getDevice(), m_commandPool, nullptr);
    }
    if (m_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device->getDevice(), m_descriptorPool, nullptr);
    }
}

void DebugUI::createDescriptorPool() {
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    
    if (vkCreateDescriptorPool(m_device->getDevice(), &pool_info, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create ImGui descriptor pool!");
    }
}

void DebugUI::createCommandBuffers() {
    VulkanDevice::QueueFamilyIndices queueFamilyIndices = m_device->findQueueFamilies(m_device->getPhysicalDevice());
    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    
    if (vkCreateCommandPool(m_device->getDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create ImGui command pool!");
    }
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    if (vkAllocateCommandBuffers(m_device->getDevice(), &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate ImGui command buffer!");
    }
}

void DebugUI::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugUI::render() {
    ImGui::Render();
}

void DebugUI::renderDrawData(VkCommandBuffer commandBuffer) {
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (draw_data && draw_data->CmdListsCount > 0) {
        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
    }
}

void DebugUI::renderDebugPanel(PerformanceStats& stats, RenderSettings& settings) {
    if (!settings.showDebugUI) return;
    
    ImGui::Begin("Debug Controls", &settings.showDebugUI);
    
    // Performance metrics
    if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("FPS: %.1f", stats.fps);
        ImGui::Text("Frame Time: %.3f ms", stats.frameTime * 1000.0f);
        ImGui::Text("CPU Time: %.3f ms", stats.cpuTime * 1000.0f);
        ImGui::Text("GPU Time: %.3f ms", stats.gpuTime * 1000.0f);
        
        // Performance graph would go here
        if (ImGui::Button("Reset Stats")) {
            stats = PerformanceStats{};
        }
    }
    
    // Rendering settings
    if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderInt("Max Raymarching Steps", &settings.maxSteps, 50, 300);
        ImGui::SliderFloat("Max Raymarching Distance", &settings.maxDistance, 50.0f, 500.0f);
        
        const char* qualityItems[] = { "Low", "Medium", "High" };
        ImGui::Combo("Quality Level", &settings.qualityLevel, qualityItems, IM_ARRAYSIZE(qualityItems));
        
        ImGui::Separator();
        ImGui::Text("Features (Simplified Mode)");
        ImGui::TextDisabled("Trees: Disabled for performance");
        ImGui::TextDisabled("Water: Disabled for performance"); 
        ImGui::TextDisabled("Clouds: Disabled for performance");
        
        ImGui::Separator();
        ImGui::Checkbox("V-Sync", &settings.enableVSync);
    }
    
    // Camera info
    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::Text("Position: (%.1f, %.1f, %.1f)", 0.0f, 0.0f, 0.0f); // Will be filled by app
        ImGui::Text("Rotation: (%.1f, %.1f)", 0.0f, 0.0f); // Will be filled by app
    }
    
    // Controls help
    if (ImGui::CollapsingHeader("Controls")) {
        ImGui::Text("WASD - Move Camera");
        ImGui::Text("Q/E - Up/Down");
        ImGui::Text("Mouse - Look Around (when captured)");
        ImGui::Text("ESC - Toggle Mouse Capture");
        ImGui::Text("F - Print FPS to Console");
        ImGui::Text("F1 - Toggle Debug UI");
        ImGui::Text("-/+ - Adjust Raymarching Steps");
    }
    
    ImGui::End();
}