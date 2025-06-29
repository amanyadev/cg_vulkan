#ifndef SWAP_CHAIN_H
#define SWAP_CHAIN_H

#include "VulkanDevice.h"
#include <vulkan/vulkan.h>
#include <vector>

class SwapChain {
public:
    SwapChain(VulkanDevice* device, VkExtent2D windowExtent);
    ~SwapChain();

    VkSwapchainKHR getSwapChain() const { return m_swapChain; }
    VkFormat getImageFormat() const { return m_imageFormat; }
    VkExtent2D getExtent() const { return m_extent; }
    const std::vector<VkImage>& getImages() const { return m_images; }
    const std::vector<VkImageView>& getImageViews() const { return m_imageViews; }

private:
    void createSwapChain();
    void createImageViews();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VulkanDevice* m_device;
    VkExtent2D m_windowExtent;

    VkSwapchainKHR m_swapChain{VK_NULL_HANDLE};
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    VkFormat m_imageFormat;
    VkExtent2D m_extent;
};

#endif // SWAP_CHAIN_H
