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
#include "viewer/GLTFViewer.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <chrono>

class VulkanApp {
public:
    void run();

private:
    const uint32_t WIDTH = 1200;
    const uint32_t HEIGHT = 800;

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
    void recreateSwapChain();
    void updatePerformanceStats();
    void processInput();
    void setupInputCallbacks();
    
    // Input callbacks
    void onMouseMove(double xpos, double ypos);
    void onMouseButton(int button, int action, int mods);
    void onScroll(double xoffset, double yoffset);
    void onKey(int key, int scancode, int action, int mods);
    void onDrop(int count, const char** paths);

    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<GraphicsPipeline> m_pipeline;
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::unique_ptr<VulkanSync> m_sync;
    std::unique_ptr<DebugUI> m_debugUI;
    std::unique_ptr<GLTFViewer> m_viewer;

    VkInstance m_instance{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};

    // Timing
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    
    // UI settings and stats
    RenderSettings m_renderSettings;
    PerformanceStats m_performanceStats;
    BackgroundSettings m_backgroundSettings;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
};

#endif // VULKANAPP_H
