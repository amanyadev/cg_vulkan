//
// Created by Aman Yadav on 6/19/25.
//

#ifndef VULKANAPP_H
#define VULKANAPP_H
#include <cstdint>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#include "VulkanDebug.h"

class VulkanApp {
public:
    void run();

private:
    const uint32_t WIDTH = 640;
    const uint32_t HEIGHT = 480;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    void initVulkan();
    void mainLoop(); // Removed static modifier
    void cleanup() const;
    void initWindow();
    void createInstance();

    // Make window and instance members of the class instead of global
    GLFWwindow* m_window{nullptr};
    VkInstance m_instance{};
    VkDebugUtilsMessengerEXT m_debugMessenger{};
};

#endif // VULKANAPP_H
