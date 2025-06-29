#include "core/VulkanSync.h"
#include <stdexcept>

VulkanSync::VulkanSync(VulkanDevice* device, uint32_t maxFramesInFlight, uint32_t imageCount)
    : m_device(device), m_maxFramesInFlight(maxFramesInFlight), m_imageCount(imageCount) {
    createSyncObjects();
}

VulkanSync::~VulkanSync() {
    cleanup();
}

void VulkanSync::createSyncObjects() {
    m_imageAvailableSemaphores.resize(m_maxFramesInFlight);
    m_renderFinishedSemaphores.resize(m_imageCount);  // One per swapchain image
    m_inFlightFences.resize(m_maxFramesInFlight);
    m_imagesInFlight.resize(m_imageCount, VK_NULL_HANDLE);  // Initialize with VK_NULL_HANDLE

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // Create frame synchronization objects
    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        if (vkCreateSemaphore(m_device->getDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_device->getDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create frame synchronization objects!");
        }
    }

    // Create per-image semaphores
    for (size_t i = 0; i < m_imageCount; i++) {
        if (vkCreateSemaphore(m_device->getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image synchronization objects!");
        }
    }
}

void VulkanSync::cleanup() {
    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        vkDestroySemaphore(m_device->getDevice(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_device->getDevice(), m_inFlightFences[i], nullptr);
    }

    for (size_t i = 0; i < m_imageCount; i++) {
        vkDestroySemaphore(m_device->getDevice(), m_renderFinishedSemaphores[i], nullptr);
    }
}

void VulkanSync::waitForFence(uint32_t frameIndex) const {
    vkWaitForFences(m_device->getDevice(), 1, &m_inFlightFences[frameIndex], VK_TRUE, UINT64_MAX);
}

void VulkanSync::resetFence(uint32_t frameIndex) const {
    vkResetFences(m_device->getDevice(), 1, &m_inFlightFences[frameIndex]);
}
