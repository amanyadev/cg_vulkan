//
// Created by Aman Yadav on 6/19/25.
//

#ifndef VULKANAPP_H
#define VULKANAPP_H
#include "VulkanDebug.h"
#include "VulkanDevice.h"
#include <cstdint>
#include <memory>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

class VulkanApp {
public:
    void run();

private:
    const uint32_t WIDTH  = 640;
    const uint32_t HEIGHT = 480;

    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    void createSurface();
    void initVulkan();
    void mainLoop();
    void cleanup();
    void initWindow();
    void createInstance();

    GLFWwindow* m_window{nullptr};
    VkInstance m_instance{};
    VkDebugUtilsMessengerEXT m_debugMessenger{};
    std::unique_ptr<VulkanDevice> m_device;
};

#endif // VULKANAPP_H
