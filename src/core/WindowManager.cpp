#include "core/WindowManager.h"
#include <stdexcept>
#include <iostream>

void WindowManager::glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

WindowManager::WindowManager(uint32_t width, uint32_t height, const char* title)
    : m_width(width), m_height(height) {
    glfwSetErrorCallback(glfwErrorCallback);

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // Required hints for Vulkan
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
#endif

    m_window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title, nullptr, nullptr);
    if (m_window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    // Verify Vulkan support
    if (!glfwVulkanSupported()) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        throw std::runtime_error("GLFW reports Vulkan is not supported!");
    }
}

WindowManager::~WindowManager() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
}

bool WindowManager::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void WindowManager::pollEvents() const {
    glfwPollEvents();
}
