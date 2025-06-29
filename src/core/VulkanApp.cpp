//
// Created by Aman Yadav on 6/19/25.
//

#include "core/VulkanApp.h"
#include "core/WindowManager.h"
#include "core/VulkanDevice.h"
#include "debug/VulkanDebug.h"
#include "rendering/SwapChain.h"
#include "rendering/GraphicsPipeline.h"
#include <iostream>
#include <stdexcept>

void VulkanApp::run() {
    std::cout << "\n=== Starting Vulkan Application ===\n" << std::endl;
    m_windowManager = std::make_unique<WindowManager>(WIDTH, HEIGHT, "Vulkan App");
    initVulkan();
    mainLoop();
    cleanup();
    std::cout << "\n=== Application Terminated Successfully ===\n" << std::endl;
}

void VulkanApp::initVulkan() {
    std::cout << "\n--- Initializing Vulkan ---" << std::endl;
    createInstance();

    if (enableValidationLayers) {
        std::cout << "Setting up debug messenger..." << std::endl;
        VulkanDebug::setupDebugMessenger(m_instance, m_debugMessenger);
    }

    std::cout << "Creating window surface..." << std::endl;
    createSurface(); // Creates and stores surface in m_surface

    std::cout << "Creating Vulkan device..." << std::endl;
    m_device = std::make_unique<VulkanDevice>(m_instance, validationLayers, enableValidationLayers);
    m_device->setSurface(m_surface); // Set our stored surface
    m_device->pickPhysicalDevice();
    m_device->createLogicalDevice();

    std::cout << "Creating swap chain..." << std::endl;
    m_swapChain = std::make_unique<SwapChain>(m_device.get(), VkExtent2D{WIDTH, HEIGHT});

    std::cout << "Creating graphics pipeline (with render pass)..." << std::endl;
    m_pipeline = std::make_unique<GraphicsPipeline>(m_device.get(), m_swapChain.get());

    std::cout << "Creating synchronization objects..." << std::endl;
    m_sync = std::make_unique<VulkanSync>(m_device.get(), MAX_FRAMES_IN_FLIGHT, m_swapChain->getImages().size());

    std::cout << "Vulkan initialization complete\n" << std::endl;
}

void VulkanApp::createSurface() {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(m_instance, m_windowManager->getWindow(), nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }

    if (surface == VK_NULL_HANDLE) {
        throw std::runtime_error("Surface creation succeeded but handle is null");
    }

    m_surface = surface; // Store surface in member variable
    std::cout << "Surface created successfully. Handle: " << surface << std::endl;
}

void VulkanApp::mainLoop() {
    std::cout << "Entering main loop..." << std::endl;
    while (!m_windowManager->shouldClose()) {
        m_windowManager->pollEvents();
        drawFrame();
    }

    // Wait for the device to finish all operations
    vkDeviceWaitIdle(m_device->getDevice());
    std::cout << "Main loop ended" << std::endl;
}

void VulkanApp::drawFrame() {
    uint32_t currentFrame = m_sync->getCurrentFrame();

    // Wait for the previous frame's fence
    m_sync->waitForFence(currentFrame);

    // Acquire the next image from the swap chain
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_device->getDevice(),
        m_swapChain->getSwapChain(),
        UINT64_MAX,
        m_sync->getImageAvailableSemaphore(currentFrame),
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    // Check if a previous frame is using this image (i.e., there is its fence to wait on)
    if (*m_sync->getImageInFlightFence(imageIndex) != VK_NULL_HANDLE) {
        vkWaitForFences(m_device->getDevice(), 1, m_sync->getImageInFlightFence(imageIndex), VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    *m_sync->getImageInFlightFence(imageIndex) = m_sync->getInFlightFence(currentFrame);

    // Reset the fence for the current frame
    m_sync->resetFence(currentFrame);

    // Record command buffer
    auto commandBuffer = m_pipeline->getCommandBuffer()->getCommandBuffer(imageIndex);

    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_sync->getImageAvailableSemaphore(currentFrame)};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {m_sync->getRenderFinishedSemaphore(imageIndex)};  // Use per-image semaphore
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, m_sync->getInFlightFence(currentFrame)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    // Present the image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapChain->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    // Advance to next frame
    m_sync->nextFrame();
}

void VulkanApp::cleanup() {
    std::cout << "\n--- Starting Cleanup ---" << std::endl;

    if (m_device) {
        std::cout << "Waiting for device to idle..." << std::endl;
        vkDeviceWaitIdle(m_device->getDevice());
    }

    m_sync.reset();  // Destroy sync objects first
    m_pipeline.reset();  // Destroy pipeline (includes framebuffer and render pass)
    m_swapChain.reset();  // Destroy swap chain
    m_device.reset();  // Destroy device

    if (m_surface != VK_NULL_HANDLE) {
        std::cout << "Destroying surface..." << std::endl;
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    if (enableValidationLayers && m_debugMessenger != VK_NULL_HANDLE) {
        std::cout << "Destroying debug messenger..." << std::endl;
        VulkanDebug::DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }

    if (m_instance != VK_NULL_HANDLE) {
        std::cout << "Destroying Vulkan instance..." << std::endl;
        vkDestroyInstance(m_instance, nullptr);
    }

    m_windowManager.reset(); // Finally destroy the window
    std::cout << "Cleanup complete" << std::endl;
}

void VulkanApp::createInstance() {
    std::cout << "Creating Vulkan instance..." << std::endl;
    if (enableValidationLayers && !VulkanDebug::checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = VulkanDebug::getRequiredExtensions(enableValidationLayers);
#ifdef __APPLE__
    extensions.push_back("VK_KHR_portability_enumeration");
    extensions.push_back("VK_KHR_get_physical_device_properties2");
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Vulkan instance!");
    }
}
