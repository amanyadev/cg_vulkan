//
// Created by Aman Yadav on 6/19/25.
//

#ifndef VULKANAPP_H
#define VULKANAPP_H

#include "VulkanDebug.h"
#include "VulkanDevice.h"
#include "WindowManager.h"
#include "SwapChain.h"
#include "GraphicsPipeline.h"
#include "Framebuffer.h"
#include <memory>

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

    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<GraphicsPipeline> m_pipeline;
    std::unique_ptr<Framebuffer> m_framebuffer;

    VkInstance m_instance{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
};

#endif // VULKANAPP_H
