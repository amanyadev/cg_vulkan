//
// Created by Aman Yadav on 6/19/25.
//

#include "core/VulkanApp.h"
#include "core/WindowManager.h"
#include "core/VulkanDevice.h"
#include "debug/VulkanDebug.h"
#include "rendering/SwapChain.h"
#include "rendering/GraphicsPipeline.h"
#include "rendering/UniformBuffer.h"
#include "ui/DebugUI.h"
#include <iostream>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

    std::cout << "Creating graphics pipeline..." << std::endl;
    m_pipeline = std::make_unique<GraphicsPipeline>(m_device.get(), m_swapChain.get());

    std::cout << "Creating synchronization objects..." << std::endl;
    m_sync = std::make_unique<VulkanSync>(m_device.get(), MAX_FRAMES_IN_FLIGHT, m_swapChain->getImages().size());

    std::cout << "Creating Debug UI..." << std::endl;
    m_debugUI = std::make_unique<DebugUI>(m_device.get(), m_swapChain.get(), m_pipeline->getRenderPass(), m_windowManager->getWindow());
    
    std::cout << "Creating Frustum Culler..." << std::endl;
    m_frustumCuller = std::make_unique<FrustumCuller>();
    
    // Skip ECS completely - focus on stunning terrain rendering performance

    // Initialize animation timer
    m_startTime = std::chrono::high_resolution_clock::now();
    m_lastFrameTime = m_startTime;
    
    // Initialize render settings for stunning terrain
    m_renderSettings.maxSteps = 64;         
    m_renderSettings.maxDistance = 150.0f;  
    m_renderSettings.enableWater = true;    
    m_renderSettings.qualityLevel = 1;      // Medium quality
    m_renderSettings.viewDistance = 250.0f; // Extended view distance
    m_renderSettings.showDebugUI = true;
    m_renderSettings.enableVSync = true;

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
        processInput();
        
        // Update performance stats
        updatePerformanceStats();
        
        // Update UI
        m_debugUI->newFrame();
        m_debugUI->renderDebugPanel(m_performanceStats, m_renderSettings);
        m_debugUI->render();
        
        // Removed ECS updates for performance - focusing on stunning terrain
        
        updateUniformBuffer();
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

    // Reset and record command buffer for this frame only
    vkResetCommandBuffer(m_pipeline->getCommandBuffer()->getCommandBuffer(imageIndex), 0);
    m_pipeline->getCommandBuffer()->recordCommandBuffer(
        imageIndex,
        m_pipeline->getRenderPass(),
        m_pipeline->getFramebuffer()->getFramebuffer(imageIndex),
        m_swapChain->getExtent(),
        m_pipeline->getPipeline(),
        m_pipeline->getPipelineLayout(),
        m_pipeline->getUniformBuffer()->getDescriptorSet(),
        m_debugUI.get()
    );
    
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

void VulkanApp::updatePerformanceStats() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_lastFrameTime).count();
    m_lastFrameTime = currentTime;
    
    if (deltaTime > 0.0f) {
        m_performanceStats.frameTime = deltaTime;
        m_performanceStats.fps = 1.0f / deltaTime;
        m_performanceStats.cpuTime = deltaTime; // Simplified for now
        m_performanceStats.gpuTime = deltaTime * 0.7f; // Estimated
    }
    
    // Sync render settings with internal variables
    m_maxSteps = m_renderSettings.maxSteps;
    m_maxDistance = m_renderSettings.maxDistance;
    m_enableWater = m_renderSettings.enableWater;
    m_qualityLevel = m_renderSettings.qualityLevel;
}

void VulkanApp::updateUniformBuffer() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_startTime).count();
    
    UniformBufferObject ubo{};
    
    // Camera and matrices
    ubo.cameraPos = m_cameraPos;
    ubo.cameraTarget = m_cameraTarget;
    ubo.time = time;
    ubo.aspectRatio = static_cast<float>(m_windowManager->getWidth()) / static_cast<float>(m_windowManager->getHeight());
    ubo.viewMatrix = glm::lookAt(m_cameraPos, m_cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.projMatrix = glm::perspective(glm::radians(45.0f), ubo.aspectRatio, 0.1f, 500.0f);
    
    // Stunning lighting setup with dynamic day/night cycle
    float sunAngle = time * 0.05f; // Slow sun movement
    ubo.sunDirection = glm::normalize(glm::vec3(
        sin(sunAngle) * 0.6f,
        0.8f + cos(sunAngle) * 0.3f,
        cos(sunAngle) * 0.4f
    ));
    ubo.sunIntensity = 2.5f;
    ubo.sunColor = glm::vec3(1.0f, 0.95f, 0.8f);
    
    // Dynamic ambient lighting based on time of day
    float dayFactor = glm::max(ubo.sunDirection.y, 0.0f);
    ubo.ambientColor = glm::mix(
        glm::vec3(0.1f, 0.15f, 0.3f), // Night
        glm::vec3(0.4f, 0.5f, 0.6f),  // Day
        dayFactor
    );
    ubo.ambientIntensity = 0.4f + dayFactor * 0.3f;
    
    // Dynamic sky colors for stunning visuals
    ubo.skyColorHorizon = glm::mix(
        glm::vec3(0.3f, 0.2f, 0.4f),  // Night horizon (purple)
        glm::vec3(1.0f, 0.7f, 0.5f),  // Day horizon (warm orange)
        dayFactor
    );
    ubo.skyColorZenith = glm::mix(
        glm::vec3(0.05f, 0.05f, 0.15f), // Night zenith (dark blue)
        glm::vec3(0.3f, 0.6f, 1.0f),     // Day zenith (bright blue)
        dayFactor
    );
    
    // Atmospheric fog
    ubo.fogColor = glm::mix(
        glm::vec3(0.2f, 0.25f, 0.4f),  // Night fog
        glm::vec3(0.8f, 0.9f, 1.0f),   // Day fog
        dayFactor
    );
    ubo.fogDensity = 0.005f;
    
    // Terrain settings for variety
    ubo.terrainScale = 1.0f;
    ubo.terrainHeight = 25.0f;
    ubo.waterLevel = -1.0f;
    ubo.enableWater = m_enableWater ? 1 : 0;
    
    // Quality settings
    ubo.qualityLevel = m_qualityLevel;
    ubo.viewDistance = 250.0f;

    // Update the uniform buffer
    m_pipeline->getUniformBuffer()->updateBuffer(ubo);
}

void VulkanApp::processInput() {
    GLFWwindow* window = m_windowManager->getWindow();
    
    // Toggle mouse capture with ESC key
    static bool escPressed = false;
    bool escCurrent = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    if (escCurrent && !escPressed) {
        m_mouseCaptured = !m_mouseCaptured;
        if (m_mouseCaptured) {
            m_windowManager->setCursorMode(GLFW_CURSOR_DISABLED);
            m_firstMouse = true; // Reset mouse to prevent jump
        } else {
            m_windowManager->setCursorMode(GLFW_CURSOR_NORMAL);
        }
    }
    escPressed = escCurrent;
    
    // Handle mouse input for camera rotation
    if (m_mouseCaptured) {
        double mouseX, mouseY;
        m_windowManager->getMousePosition(&mouseX, &mouseY);
        
        if (m_firstMouse) {
            m_lastMouseX = mouseX;
            m_lastMouseY = mouseY;
            m_firstMouse = false;
        }
        
        double deltaX = mouseX - m_lastMouseX;
        double deltaY = m_lastMouseY - mouseY; // Reversed since y-coordinates go from bottom to top
        
        m_lastMouseX = mouseX;
        m_lastMouseY = mouseY;
        
        // Update camera angles
        m_cameraYaw += static_cast<float>(deltaX) * m_mouseSensitivity;
        m_cameraPitch += static_cast<float>(deltaY) * m_mouseSensitivity;
        
        // Constrain pitch to prevent camera flipping
        const float maxPitch = 1.5f; // ~86 degrees
        if (m_cameraPitch > maxPitch) m_cameraPitch = maxPitch;
        if (m_cameraPitch < -maxPitch) m_cameraPitch = -maxPitch;
        
        // Update camera target based on yaw and pitch
        glm::vec3 direction;
        direction.x = cos(m_cameraYaw) * cos(m_cameraPitch);
        direction.y = sin(m_cameraPitch);
        direction.z = sin(m_cameraYaw) * cos(m_cameraPitch);
        m_cameraTarget = m_cameraPos + glm::normalize(direction) * 5.0f;
    }
    
    // Camera movement speed
    const float cameraSpeed = 0.1f;
    
    // Camera movement with WASD
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm::vec3 forward = glm::normalize(m_cameraTarget - m_cameraPos);
        m_cameraPos += cameraSpeed * forward;
        m_cameraTarget += cameraSpeed * forward;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm::vec3 forward = glm::normalize(m_cameraTarget - m_cameraPos);
        m_cameraPos -= cameraSpeed * forward;
        m_cameraTarget -= cameraSpeed * forward;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm::vec3 forward = glm::normalize(m_cameraTarget - m_cameraPos);
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_cameraPos -= right * cameraSpeed;
        m_cameraTarget -= right * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm::vec3 forward = glm::normalize(m_cameraTarget - m_cameraPos);
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_cameraPos += right * cameraSpeed;
        m_cameraTarget += right * cameraSpeed;
    }
    
    // Camera height with Q/E
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        m_cameraPos.y -= cameraSpeed;
        m_cameraTarget.y -= cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        m_cameraPos.y += cameraSpeed;
        m_cameraTarget.y += cameraSpeed;
    }
    
    
    // Performance control hotkeys (with debouncing)
    static bool keys[10] = {false}; // Track key states for debouncing
    
    // Toggle quality level with '1'
    bool key1 = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
    if (key1 && !keys[0]) {
        m_qualityLevel = (m_qualityLevel + 1) % 3;
        std::cout << "Quality: " << (m_qualityLevel == 0 ? "Low" : (m_qualityLevel == 1 ? "Medium" : "High")) << std::endl;
    }
    keys[0] = key1;
    
    // Toggle water with '2'
    bool key2 = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
    if (key2 && !keys[1]) {
        m_enableWater = !m_enableWater;
        std::cout << "Water: " << (m_enableWater ? "ON" : "OFF") << std::endl;
    }
    keys[1] = key2;
    
    // Toggle view distance with '3'
    bool key3 = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
    if (key3 && !keys[2]) {
        m_renderSettings.viewDistance = (m_renderSettings.viewDistance > 250.0f) ? 150.0f : 350.0f;
        std::cout << "View Distance: " << m_renderSettings.viewDistance << std::endl;
    }
    keys[2] = key3;
    
    // Adjust max steps with '-' and '='
    bool keyMinus = glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS;
    if (keyMinus && !keys[3]) {
        m_maxSteps = glm::max(20, m_maxSteps - 10);
        std::cout << "Max Steps: " << m_maxSteps << std::endl;
    }
    keys[3] = keyMinus;
    
    bool keyEqual = glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS;
    if (keyEqual && !keys[4]) {
        m_maxSteps = glm::min(200, m_maxSteps + 10);
        std::cout << "Max Steps: " << m_maxSteps << std::endl;
    }
    keys[4] = keyEqual;
    
    // Print FPS with 'F'
    bool keyF = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
    if (keyF && !keys[5]) {
        std::cout << "FPS: " << m_performanceStats.fps << " | Frame Time: " << m_performanceStats.frameTime * 1000.0f << "ms" << std::endl;
        std::cout << "Camera: (" << m_cameraPos.x << ", " << m_cameraPos.y << ", " << m_cameraPos.z << ")" << std::endl;
    }
    keys[5] = keyF;
    
    // Toggle UI with 'F1'
    bool keyF1 = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;
    if (keyF1 && !keys[6]) {
        m_renderSettings.showDebugUI = !m_renderSettings.showDebugUI;
        std::cout << "Debug UI: " << (m_renderSettings.showDebugUI ? "ON" : "OFF") << std::endl;
    }
    keys[6] = keyF1;
}

