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
#include <glm/glm.hpp>

struct PerformanceStats {
    float frameTime = 0.0f;
    float fps = 0.0f;
    int trianglesRendered = 0;
    float cpuTime = 0.0f;
    float gpuTime = 0.0f;
    int qualityLevel = 2;
};

struct RenderSettings {
    int maxSteps = 150;
    float maxDistance = 200.0f;
    bool enableWater = true;
    int qualityLevel = 2; // 0=low, 1=med, 2=high
    bool showDebugUI = true;
    bool enableVSync = true;
    
    // Visual settings for stunning terrain
    float viewDistance = 250.0f;
    float fogDensity = 0.005f;
    glm::vec3 skyHorizon = glm::vec3(1.0f, 0.7f, 0.5f);
    glm::vec3 skyZenith = glm::vec3(0.3f, 0.6f, 1.0f);
    glm::vec3 sunColor = glm::vec3(1.0f, 0.95f, 0.8f);
    float sunIntensity = 2.5f;
    float timeSpeed = 0.05f;
    float timeOffset = 0.0f;
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