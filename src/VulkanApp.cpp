//
// Created by Aman Yadav on 6/19/25.
//

#include "../include/VulkanApp.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <stdexcept>

static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void VulkanApp::run() {
    std::cout << "\n=== Starting Vulkan Application ===\n" << std::endl;
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
    std::cout << "\n=== Application Terminated Successfully ===\n" << std::endl;
}

void VulkanApp::initWindow() {
    std::cout << "Initializing GLFW window..." << std::endl;
    glfwSetErrorCallback(glfwErrorCallback);

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan App", nullptr, nullptr);
    if (m_window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    std::cout << "Window created successfully: " << WIDTH << "x" << HEIGHT << std::endl;
}

void VulkanApp::createSurface() {
    std::cout << "\nCreating window surface..." << std::endl;
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }

    if (surface == VK_NULL_HANDLE) {
        throw std::runtime_error("Surface creation succeeded but handle is null");
    }

    std::cout << "Surface created successfully. Handle: " << surface << std::endl;
    m_device->setSurface(surface);

    // Verify surface was set in device
    if (m_device->getSurface() != surface) {
        throw std::runtime_error("Surface was not properly set in device");
    }
    std::cout << "Surface successfully set in device" << std::endl;
}

void VulkanApp::initVulkan() {
    std::cout << "\n--- Initializing Vulkan ---" << std::endl;
    createInstance();
    if (enableValidationLayers) {
        std::cout << "Setting up debug messenger..." << std::endl;
        VulkanDebug::setupDebugMessenger(m_instance, m_debugMessenger);
    }
    std::cout << "Creating Vulkan device..." << std::endl;
    m_device = std::make_unique<VulkanDevice>(m_instance, validationLayers, enableValidationLayers);
    createSurface();  // Create surface before picking physical device
    m_device->pickPhysicalDevice();
    m_device->createLogicalDevice();
    std::cout << "Vulkan initialization complete\n" << std::endl;
}

void VulkanApp::mainLoop() {
    std::cout << "Entering main loop..." << std::endl;
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
    std::cout << "Main loop ended" << std::endl;
}

void VulkanApp::cleanup() {
    std::cout << "\n--- Starting Cleanup ---" << std::endl;
    if (m_device) {
        std::cout << "Waiting for device to idle..." << std::endl;
        vkDeviceWaitIdle(m_device->getDevice());
        if (m_device->getSurface() != VK_NULL_HANDLE) {
            std::cout << "Destroying surface..." << std::endl;
            vkDestroySurfaceKHR(m_instance, m_device->getSurface(), nullptr);
        }
        std::cout << "Destroying device..." << std::endl;
        m_device.reset();
    }

    if (enableValidationLayers) {
        std::cout << "Destroying debug messenger..." << std::endl;
        VulkanDebug::DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }

    std::cout << "Destroying Vulkan instance..." << std::endl;
    vkDestroyInstance(m_instance, nullptr);

    if (m_window) {
        std::cout << "Cleaning up GLFW..." << std::endl;
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
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
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"}; // Move outside if block
    if (enableValidationLayers) {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        VulkanDebug::populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext             = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
    std::cout << "Vulkan instance created successfully" << std::endl;
}
