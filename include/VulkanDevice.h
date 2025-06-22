#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

class VulkanDevice {
public:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        bool isComplete() {
            return graphicsFamily.has_value();
        }
    };

    VulkanDevice(VkInstance instance, const std::vector<const char*>& validationLayers, bool enableValidationLayers);
    ~VulkanDevice();

    void createLogicalDevice();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    // Getters
    VkDevice getDevice() const { return m_logicalDevice; }
    VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
    VkQueue getGraphicsQueue() const { return m_graphicsQueue; }

private:
    static int rateDeviceSuitability(VkPhysicalDevice device);

    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_logicalDevice{VK_NULL_HANDLE};
    VkQueue m_graphicsQueue{};

    const std::vector<const char*>& m_validationLayers;
    bool m_enableValidationLayers;
};

#endif // VULKAN_DEVICE_H
