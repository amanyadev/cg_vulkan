#pragma once

#include "core/VulkanDevice.h"
#include "rendering/SwapChain.h"
#include "rendering/RenderPass.h"
#include "rendering/UniformBuffer.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <memory>

struct PerformanceStats {
    float frameTime = 0.0f;
    float fps = 0.0f;
    int trianglesRendered = 0;
    float cpuTime = 0.0f;
    float gpuTime = 0.0f;
};

struct RenderSettings {
    int maxSteps = 150;
    float maxDistance = 200.0f;
    bool enableTrees = true;
    bool enableWater = true;
    bool enableClouds = true;
    int qualityLevel = 2; // 0=low, 1=med, 2=high
    float treeDistance = 50.0f;
    bool showDebugUI = true;
    bool enableVSync = true;
};

class DebugUI {
public:
    DebugUI(VulkanDevice* device, SwapChain* swapChain, VkRenderPass renderPass, GLFWwindow* window);
    ~DebugUI();

    void newFrame();
    void render();
    void renderDebugPanel(PerformanceStats& stats, RenderSettings& settings);
    void renderDrawData(VkCommandBuffer commandBuffer);
    
    VkCommandBuffer getCommandBuffer() const { return m_commandBuffer; }
    bool wantsCaptureMouse() const { return ImGui::GetIO().WantCaptureMouse; }
    bool wantsCaptureKeyboard() const { return ImGui::GetIO().WantCaptureKeyboard; }

private:
    void createDescriptorPool();
    void createCommandBuffers();

    VulkanDevice* m_device;
    SwapChain* m_swapChain;
    VkRenderPass m_renderPass;
    GLFWwindow* m_window;
    
    VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};
    VkCommandPool m_commandPool{VK_NULL_HANDLE};
    VkCommandBuffer m_commandBuffer{VK_NULL_HANDLE};
    
    bool m_initialized{false};
};