#ifndef VULKAN_DEBUG_H
#define VULKAN_DEBUG_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class VulkanDebug {
public:
    static bool checkValidationLayerSupport();
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT& debugMessenger);
    static std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    static void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);

private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    static const std::vector<const char*> validationLayers;
};

#endif // VULKAN_DEBUG_H
