//
// Created by Aman Yadav on 6/19/25.
//

#ifndef VULKANAPP_H
#define VULKANAPP_H
#include "VulkanDebug.h"
#include <cstdint>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

class VulkanApp {
public:
    void run();

private:
    const uint32_t WIDTH  = 640;
    const uint32_t HEIGHT = 480;
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        bool isComplete() {
            return graphicsFamily.has_value();
        }
    };
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    void createLogicalDevice();
    void initVulkan();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    static int rateDeviceSuitability(VkPhysicalDevice device);
    void mainLoop(); // Removed static modifier
    void cleanup() const;
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void initWindow();
    void createInstance();

    // Make window and instance members of the class instead of global
    GLFWwindow* m_window{nullptr};
    VkInstance m_instance{};
    VkDebugUtilsMessengerEXT m_debugMessenger{};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice logicalDevice{VK_NULL_HANDLE};
    VkQueue graphicsQueue{};
};

#endif // VULKANAPP_H
