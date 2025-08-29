//
// Created by Aman Yadav on 6/19/25.
//

#ifndef VULKANAPP_H
#define VULKANAPP_H

#include "core/WindowManager.h"
#include "core/VulkanDevice.h"
#include "core/VulkanSync.h"
#include "debug/VulkanDebug.h"
#include "rendering/SwapChain.h"
#include "rendering/GraphicsPipeline.h"
#include "ui/DebugUI.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <chrono>

class VulkanApp {
public:
    void run();

private:
    const uint32_t WIDTH = 640;
    const uint32_t HEIGHT = 480;

    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    void initVulkan();
    void mainLoop();
    void cleanup();
    void createInstance();
    void createSurface();
    void createFramebuffers();
    void drawFrame();
    void updateUniformBuffer();
    void updatePerformanceStats();
    void processInput();

    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<GraphicsPipeline> m_pipeline;
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::unique_ptr<VulkanSync> m_sync;
    std::unique_ptr<DebugUI> m_debugUI;

    VkInstance m_instance{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};

    // Enhanced camera and animation state
    glm::vec3 m_cameraPos{0.0f, 15.0f, -25.0f};
    glm::vec3 m_cameraTarget{0.0f, 5.0f, 0.0f};
    glm::vec3 m_cameraVelocity{0.0f, 0.0f, 0.0f};
    float m_cameraYaw{0.0f};
    float m_cameraPitch{0.0f};
    bool m_firstMouse{true};
    double m_lastMouseX{400.0};
    double m_lastMouseY{300.0};
    float m_mouseSensitivity{0.002f};
    bool m_mouseCaptured{false};
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    
    // Performance settings
    int m_maxSteps{80};          // Reduced from 150 for better performance
    float m_maxDistance{150.0f}; // Reduced from 200 for better performance
    bool m_enableTrees{true};
    bool m_enableWater{true};
    bool m_enableClouds{true};
    int m_qualityLevel{1};       // 0=low, 1=med, 2=high
    float m_treeDistance{30.0f}; // Reduced from 50 for better performance
    
    // Frame timing
    float m_frameTime{0.0f};
    float m_fps{0.0f};
    
    // UI settings and stats
    RenderSettings m_renderSettings;
    PerformanceStats m_performanceStats;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
};

#endif // VULKANAPP_H
