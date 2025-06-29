#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

class VulkanDevice {
public:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    VulkanDevice(VkInstance instance, const std::vector<const char*>& validationLayers, bool enableValidationLayers);
    ~VulkanDevice();

    void createLogicalDevice();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    void setSurface(VkSurfaceKHR newSurface) {
        surface = newSurface;
    }

    // Getters
    [[nodiscard]] VkDevice getDevice() const {
        return m_logicalDevice;
    }
    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const {
        return m_physicalDevice;
    }
    [[nodiscard]] VkQueue getGraphicsQueue() const {
        return m_graphicsQueue;
    }
    [[nodiscard]] VkQueue getPresentQueue() const {
        return m_presentQueue;
    }
    [[nodiscard]] VkSurfaceKHR getSurface() const {
        return surface;
    }

private:
    static int rateDeviceSuitability(VkPhysicalDevice device);

    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_logicalDevice{VK_NULL_HANDLE};
    VkQueue m_graphicsQueue{};
    VkQueue m_presentQueue{};
    VkSurfaceKHR surface{VK_NULL_HANDLE};

    const std::vector<const char*>& m_validationLayers;
    bool m_enableValidationLayers;
};

#endif // VULKAN_DEVICE_H
