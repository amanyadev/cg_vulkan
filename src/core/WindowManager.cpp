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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
#endif

    m_window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title, nullptr, nullptr);
    if (m_window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    // Set user pointer for callbacks
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    
    // Set input callbacks
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetDropCallback(m_window, dropCallback);

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

void WindowManager::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto* windowManager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    windowManager->m_framebufferResized = true;
    windowManager->updateSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}

// Mouse control implementations
void WindowManager::getMousePosition(double* xpos, double* ypos) const {
    glfwGetCursorPos(m_window, xpos, ypos);
}

void WindowManager::setMousePosition(double xpos, double ypos) const {
    glfwSetCursorPos(m_window, xpos, ypos);
}

bool WindowManager::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void WindowManager::setCursorMode(int mode) const {
    glfwSetInputMode(m_window, GLFW_CURSOR, mode);
}

// GLFW input callback implementations
void WindowManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* windowManager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (windowManager && windowManager->m_mouseMoveCallback) {
        windowManager->m_mouseMoveCallback(xpos, ypos);
    }
}

void WindowManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* windowManager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (windowManager && windowManager->m_mouseButtonCallback) {
        windowManager->m_mouseButtonCallback(button, action, mods);
    }
}

void WindowManager::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* windowManager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (windowManager && windowManager->m_scrollCallback) {
        windowManager->m_scrollCallback(xoffset, yoffset);
    }
}

void WindowManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* windowManager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (windowManager && windowManager->m_keyCallback) {
        windowManager->m_keyCallback(key, scancode, action, mods);
    }
}

void WindowManager::dropCallback(GLFWwindow* window, int count, const char** paths) {
    auto* windowManager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (windowManager && windowManager->m_dropCallback) {
        windowManager->m_dropCallback(count, paths);
    }
}
