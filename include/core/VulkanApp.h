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
#include <memory>
#include <vector>

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

    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<GraphicsPipeline> m_pipeline;
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::unique_ptr<VulkanSync> m_sync;

    VkInstance m_instance{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
};

#endif // VULKANAPP_H
