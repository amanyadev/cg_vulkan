//
// Created by Aman Yadav on 6/19/25.
//

#include "../include/VulkanApp.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void VulkanApp::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void VulkanApp::initWindow() {
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
}

void VulkanApp::initVulkan() {
    createInstance();
    if (enableValidationLayers) {
        VulkanDebug::setupDebugMessenger(m_instance, m_debugMessenger);
    }
}

void VulkanApp::createInstance() {
    if (enableValidationLayers && !VulkanDebug::checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = VulkanDebug::getRequiredExtensions(enableValidationLayers);
#ifdef __APPLE__
    extensions.push_back("VK_KHR_portability_enumeration");
    extensions.push_back("VK_KHR_get_physical_device_properties2");
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"}; // Move outside if block
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        VulkanDebug::populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void VulkanApp::mainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

void VulkanApp::cleanup() const {
    if (enableValidationLayers) {
        VulkanDebug::DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }

    vkDestroyInstance(m_instance, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}
