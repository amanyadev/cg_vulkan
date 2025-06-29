#pragma once

#include "core/VulkanDevice.h"
#include <vulkan/vulkan.h>
#include <vector>

class VulkanSync {
public:
    VulkanSync(VulkanDevice* device, uint32_t maxFramesInFlight, uint32_t imageCount);
    ~VulkanSync();

    // Get synchronization objects
    VkSemaphore getImageAvailableSemaphore(uint32_t frameIndex) const { return m_imageAvailableSemaphores[frameIndex]; }
    VkSemaphore getRenderFinishedSemaphore(uint32_t imageIndex) const { return m_renderFinishedSemaphores[imageIndex]; }
    VkFence getInFlightFence(uint32_t frameIndex) const { return m_inFlightFences[frameIndex]; }
    VkFence* getImageInFlightFence(uint32_t imageIndex) { return &m_imagesInFlight[imageIndex]; }

    // Frame management
    void nextFrame() { m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight; }
    uint32_t getCurrentFrame() const { return m_currentFrame; }

    // Fence operations
    void waitForFence(uint32_t frameIndex) const;
    void resetFence(uint32_t frameIndex) const;

private:
    void createSyncObjects();
    void cleanup();

    VulkanDevice* m_device;
    uint32_t m_maxFramesInFlight;
    uint32_t m_imageCount;
    uint32_t m_currentFrame{0};

    std::vector<VkSemaphore> m_imageAvailableSemaphores;  // One per frame
    std::vector<VkSemaphore> m_renderFinishedSemaphores;  // One per swapchain image
    std::vector<VkFence> m_inFlightFences;                // One per frame
    std::vector<VkFence> m_imagesInFlight;                // One per swapchain image
};
