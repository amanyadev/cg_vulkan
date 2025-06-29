#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class WindowManager {
public:
    WindowManager(uint32_t width, uint32_t height, const char* title);
    ~WindowManager();

    GLFWwindow* getWindow() const { return m_window; }
    bool shouldClose() const;
    void pollEvents() const;

    uint32_t getWidth() const { return m_width; }
    uint32_t getHeight() const { return m_height; }

private:
    GLFWwindow* m_window;
    uint32_t m_width;
    uint32_t m_height;

    static void glfwErrorCallback(int error, const char* description);
};

#endif // WINDOW_MANAGER_H
