#include "../include/VulkanDevice.h"

#include <iostream>
#include <set>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

VulkanDevice::VulkanDevice(
    VkInstance instance, const std::vector<const char*>& validationLayers, bool enableValidationLayers)
    : m_instance(instance), m_validationLayers(validationLayers), m_enableValidationLayers(enableValidationLayers) {
    std::cout << "VulkanDevice: Initializing..." << std::endl;
}

VulkanDevice::~VulkanDevice() {
    if (m_logicalDevice != VK_NULL_HANDLE) {
        std::cout << "VulkanDevice: Destroying logical device" << std::endl;
        vkDestroyDevice(m_logicalDevice, nullptr);
    }
}

void VulkanDevice::createLogicalDevice() {
    std::cout << "VulkanDevice: Creating logical device..." << std::endl;
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    std::cout << "VulkanDevice: Setting up " << uniqueQueueFamilies.size() << " queue(s)" << std::endl;

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos    = queueCreateInfos.data();
    createInfo.pEnabledFeatures     = &deviceFeatures;

    std::vector<const char*> deviceExtensions;
#ifdef __APPLE__
    deviceExtensions.push_back("VK_KHR_portability_subset");
#endif
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    std::cout << "VulkanDevice: Enabling " << deviceExtensions.size() << " device extension(s)" << std::endl;
    for (const auto& ext : deviceExtensions) {
        std::cout << "  - " << ext << std::endl;
    }

    createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (m_enableValidationLayers) {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    std::cout << "VulkanDevice: Retrieving queue handles..." << std::endl;
    vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue);
    std::cout << "VulkanDevice: Logical device created successfully" << std::endl;
}

void VulkanDevice::pickPhysicalDevice() {
    std::cout << "VulkanDevice: Selecting physical device..." << std::endl;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::cout << "VulkanDevice: Found " << deviceCount << " Vulkan capable device(s)" << std::endl;
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        std::cout << "VulkanDevice: Evaluating device: " << deviceProperties.deviceName << std::endl;

        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            std::cout << "VulkanDevice: Selected device: " << deviceProperties.deviceName << std::endl;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device) {
    std::cout << "VulkanDevice: Checking device suitability..." << std::endl;
    QueueFamilyIndices indices = findQueueFamilies(device);

    if (!indices.isComplete()) {
        std::cout << "VulkanDevice: Device missing required queue families" << std::endl;
        return false;
    }

    // Verify surface support capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities) != VK_SUCCESS) {
        std::cout << "VulkanDevice: Device does not support surface capabilities" << std::endl;
        return false;
    }

    // Check for surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount == 0) {
        std::cout << "VulkanDevice: Device does not support any surface formats" << std::endl;
        return false;
    }

    // Check for present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount == 0) {
        std::cout << "VulkanDevice: Device does not support any presentation modes" << std::endl;
        return false;
    }

    std::cout << "VulkanDevice: Device supports " << formatCount << " surface format(s) and " << presentModeCount
              << " present mode(s)" << std::endl;
    return true;
}

int VulkanDevice::rateDeviceSuitability(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    int score = 0;

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }
    score += static_cast<int>(deviceProperties.limits.maxImageDimension2D);

    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    return score;
}

VulkanDevice::QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device) {
    std::cout << "VulkanDevice: Finding queue families..." << std::endl;
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    std::cout << "VulkanDevice: Examining " << queueFamilyCount << " queue familie(s)" << std::endl;
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            std::cout << "VulkanDevice: Found graphics queue family at index " << i << std::endl;
        }

        VkBool32 presentSupport = false;
        if (surface != VK_NULL_HANDLE) {
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
                std::cout << "VulkanDevice: Found present queue family at index " << i << std::endl;
            }
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }

    return indices;
}

VulkanDevice::SwapChainSupportDetails VulkanDevice::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}
VkPresentModeKHR VulkanDevice::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& mode : availablePresentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode; // Mailbox is preferred for low latency
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR; // Fallback to FIFO
}
VkSurfaceFormatKHR VulkanDevice::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& format : availableFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format; // Preferred format
        }
    }
    return availableFormats[0]; // Fallback to the first available format
}
